/* The following file was heavily referenced/copied/translated from C++ to write this file:

https://github.com/charmedlabs/pixy2/blob/master/src/host/arduino/libraries/Pixy2/TPixy2.h

The translation is not fully complete. It only implements the necesary functionality for the 
project it was created for.
*/

#include "pixy_spi.h"
#include "stdio.h"

#define PIXY_DEFAULT_ARGVAL                  0x80000000
#define PIXY_BUFFERSIZE                      0x104
#define PIXY_CHECKSUM_SYNC                   0xc1af
#define PIXY_NO_CHECKSUM_SYNC                0xc1ae
#define PIXY_SEND_HEADER_SIZE                4
#define PIXY_MAX_PROGNAME                    33

#define PIXY_TYPE_REQUEST_CHANGE_PROG        0x02
#define PIXY_TYPE_REQUEST_RESOLUTION         0x0c
#define PIXY_TYPE_RESPONSE_RESOLUTION        0x0d
#define PIXY_TYPE_REQUEST_VERSION            0x0e
#define PIXY_TYPE_RESPONSE_VERSION           0x0f
#define PIXY_TYPE_RESPONSE_RESULT            0x01
#define PIXY_TYPE_RESPONSE_ERROR             0x03
#define PIXY_TYPE_REQUEST_BRIGHTNESS         0x10
#define PIXY_TYPE_REQUEST_SERVO              0x12
#define PIXY_TYPE_REQUEST_LED                0x14
#define PIXY_TYPE_REQUEST_LAMP               0x16
#define PIXY_TYPE_REQUEST_FPS                0x18

#define PIXY_RESULT_OK                       0
#define PIXY_RESULT_ERROR                    -1
#define PIXY_RESULT_BUSY                     -2
#define PIXY_RESULT_CHECKSUM_ERROR           -3
#define PIXY_RESULT_TIMEOUT                  -4
#define PIXY_RESULT_BUTTON_OVERRIDE          -5
#define PIXY_RESULT_PROG_CHANGING            -6

// RC-servo values
#define PIXY_RCS_MIN_POS                     0
#define PIXY_RCS_MAX_POS                     1000L
#define PIXY_RCS_CENTER_POS                  ((PIXY_RCS_MAX_POS-PIXY_RCS_MIN_POS)/2)

typedef struct Version {
  uint16_t hardware;
  uint8_t firmwareMajor;
  uint8_t firmwareMinor;
  uint16_t firmwareBuild;
  char firmwareType[10];   
} Version;

uint8_t m_buf[PIXY_BUFFERSIZE];
uint8_t *m_bufPayload = m_buf + PIXY_SEND_HEADER_SIZE;
uint8_t m_type;
uint8_t m_length;
bool m_cs;
uint8_t offset;

Version *pixyVersion;
uint16_t frameWidth;
uint16_t frameHeight;

int16_t getSync() {
  uint8_t i, j;
  int16_t res;
  uint16_t c, cprev, cmid;
  
  
  // parse bytes until we find sync
  for(i = j = 0, cprev = 0; true; i++) {
    // NOTE: Reading two bytes at a time instead of 1. Implementation from pixy2 docs only reads 1 byte at a time
    // but there were issues doing so with the spi library used for this project.
    // Reading two bytes at a time could cause issues if further functionality is added, but it works for the 
    // functionality required at the time of writing this translation.
    res = pixy_spi_recv(&c, 2, NULL);
    if (res >= PIXY_RESULT_OK) {
      cmid = (cprev >> 8) + (c << 8);
      if (c == PIXY_CHECKSUM_SYNC) {
        m_cs = true;
        offset = 0;
        return PIXY_RESULT_OK;
      } 
      if (c == PIXY_NO_CHECKSUM_SYNC) {
        m_cs = false;
        offset = 0;
        return PIXY_RESULT_OK;
      }
      if (cmid == PIXY_CHECKSUM_SYNC) {
        m_cs = true;
        m_type = c >> 8;
        offset = 1;
        return PIXY_RESULT_OK;
      } 
      if (cmid == PIXY_NO_CHECKSUM_SYNC) {
        m_cs = false;
        m_type = c >> 8;
        offset = 1;
        return PIXY_RESULT_OK;
      }
      cprev = c;
    }
	  // If we've read some bytes and no sync, then wait and try again.
	  // And do that several more times before we give up.  
	  // Pixy guarantees to respond within 100us.
    if (i >= 4) {
      if (j >= 4) {
        return PIXY_RESULT_ERROR;
      }
      pixy_spi_delayus(25); 
      j++;
      i = 0;
    }
  }
}


int16_t recvPacket() {
  uint16_t csCalc, csSerial;
  int16_t res;
  
  //printf("Sync...\n");
  res = getSync();
  
  if (res < 0) {
    return res;
  }

  if (m_cs) {
    
    res = pixy_spi_recv(m_buf, 4 - offset, NULL);
    if (res < 0) {
      return res;
    }

    if (offset != 1) {
      m_type = m_buf[0];
    }
    
    m_length = m_buf[1 - offset];

    csSerial = *(uint16_t *)&m_buf[2 - offset];

    res = pixy_spi_recv(m_buf, m_length, &csCalc);
    if (res < 0) {
      return res;
    }

    if (csSerial!=csCalc)
    {
      return PIXY_RESULT_CHECKSUM_ERROR;
    }
  } else {  
    res = pixy_spi_recv(m_buf, 2, NULL);
    if (res < 0) {
      return res;
    }

    if (offset != 1) {
      m_type = m_buf[0];
    }
    m_length = m_buf[1 - offset];

    res = pixy_spi_recv(m_buf, m_length, NULL);
    if (res < 0) {
      return res;
    }
  }
  return PIXY_RESULT_OK;
}


int16_t sendPacket() {
  // write header info at beginnig of buffer
  m_buf[0] = PIXY_NO_CHECKSUM_SYNC&0xff;
  m_buf[1] = PIXY_NO_CHECKSUM_SYNC>>8;
  m_buf[2] = m_type;
  m_buf[3] = m_length;
  // send whole thing -- header and data in one call
  return pixy_spi_send(m_buf, m_length+PIXY_SEND_HEADER_SIZE);
}

int8_t getVersion() {
  m_length = 0;
  m_type = PIXY_TYPE_REQUEST_VERSION;
  sendPacket();
  if (recvPacket() == 0) {   
    if (m_type == PIXY_TYPE_RESPONSE_VERSION) {
      pixyVersion = (Version *)m_buf;
      return m_length;
    }
    else if (m_type == PIXY_TYPE_RESPONSE_ERROR) {
      return PIXY_RESULT_BUSY;
    }
  }
  return PIXY_RESULT_ERROR;  // some kind of bitstream error
}

void printVersion() {
	if (pixyVersion != NULL) {
	    char buf[64];
        sprintf(buf, "hardware ver: 0x%x firmware ver: %d.%d.%d %s", pixyVersion->hardware, 
    	                                                             pixyVersion->firmwareMajor,
    	                                                             pixyVersion->firmwareMinor,
    	                                                             pixyVersion->firmwareBuild, 
    	                                                             pixyVersion->firmwareType);
        printf("%s\n", buf);
    } else {
    	printf("Version unavailable. Call getVersion to record Pixy version");
    }
}

int8_t getResolution() {
  m_length = 1;
  m_bufPayload[0] = 0; // for future types of queries
  m_type = PIXY_TYPE_REQUEST_RESOLUTION;
  sendPacket();
  if (recvPacket() == 0) {   
    if (m_type == PIXY_TYPE_RESPONSE_RESOLUTION) {
      frameWidth = *(uint16_t *)m_buf;
      frameHeight = *(uint16_t *)(m_buf+sizeof(uint16_t));
      return PIXY_RESULT_OK; // success
    }
  }
  printf("Resolution retreival error\n");
  return PIXY_RESULT_ERROR;  // some kind of bitstream error

} 

int8_t pixyInit(uint32_t arg) {

  pixyVersion = NULL;
  frameWidth = 0;
  frameHeight = 0;
  
  int8_t res = pixy_spi_open(arg);
  if (res < 0) {
    return res;
  }
  
  int delay_count = 0;
  while (delay_count < 5000) {
  	if (getVersion() >= 0) {
  		getResolution();
  		return PIXY_RESULT_OK;
  	}
  	pixy_spi_delayms(1);
  	delay_count++;
  }
  
  return PIXY_RESULT_TIMEOUT;
}

void pixyUninit() {
  pixy_spi_close();
}

int8_t setLED(uint8_t r, uint8_t g, uint8_t b) {
	uint32_t res;
  m_bufPayload[0] = r;
  m_bufPayload[1] = g;
  m_bufPayload[2] = b;
  m_length = 3;
  m_type = PIXY_TYPE_REQUEST_LED;
  sendPacket();
  if (recvPacket()==0 && m_type==PIXY_TYPE_RESPONSE_RESULT && m_length==4)
  {
    res = *(uint32_t *)m_buf;
    return (int8_t)res;	
  }
  else
    return PIXY_RESULT_ERROR;  // some kind of bitstream error
    
}

int8_t setLamp(uint8_t upper, uint8_t lower) {
  uint32_t res;
  
  m_bufPayload[0] = upper;
  m_bufPayload[1] = lower;
  m_length = 2;
  m_type = PIXY_TYPE_REQUEST_LAMP;
  sendPacket();
  if (recvPacket()==0 && m_type==PIXY_TYPE_RESPONSE_RESULT && m_length==4) {
    res = *(uint32_t *)m_buf;
    return (int8_t)res;	
  } else {
      return PIXY_RESULT_ERROR;  // some kind of bitstream error	
  }
}

