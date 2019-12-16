/* The following file was heavily referenced/copied/translated from C++ to write this file:

https://github.com/charmedlabs/pixy2/blob/master/src/host/arduino/libraries/Pixy2/Pixy2CCC.h
*/

#include "pixy.h"

#define CCC_MAX_SIGNATURE                   7

#define CCC_RESPONSE_BLOCKS                 0x21
#define CCC_REQUEST_BLOCKS                  0x20

// Defines for sigmap:
// You can bitwise "or" these together to make a custom sigmap.
// For example if you're only interested in receiving blocks
// with signatures 1 and 5, you could use a sigmap of 
// PIXY_SIG1 | PIXY_SIG5
#define CCC_SIG1                     1 
#define CCC_SIG2                     2
#define CCC_SIG3                     4
#define CCC_SIG4                     8
#define CCC_SIG5                     16
#define CCC_SIG6                     32
#define CCC_SIG7                     64
#define CCC_COLOR_CODES              128

#define CCC_SIG_ALL                  0xff // all bits or'ed together

typedef struct Block 
{ 
  uint16_t m_signature;
  uint16_t m_x;
  uint16_t m_y;
  uint16_t m_width;
  uint16_t m_height;
  int16_t m_angle;
  uint8_t m_index;
  uint8_t m_age;
} Block;

uint8_t numBlocks = 0;
Block *blocks = NULL;


// Slightly modified from the implementation given in the pixy2 docs
int8_t getBlocks(bool wait, uint8_t sigmap, uint8_t maxBlocks) {

  numBlocks = 0;

  // fill in request data
  m_bufPayload[0] = sigmap;
  m_bufPayload[1] = maxBlocks;
  m_length = 2;
  m_type = CCC_REQUEST_BLOCKS;

  // send request
  sendPacket();
  if (recvPacket()==0)
  {
    if (m_type==CCC_RESPONSE_BLOCKS)
    {
      blocks = (Block *)m_buf;
      numBlocks = m_length/sizeof(Block);
      return numBlocks;
    }
  // deal with busy and program changing states from Pixy (we'll wait)
    else if (m_type==PIXY_TYPE_RESPONSE_ERROR)
    {
      if ((int8_t)m_buf[0]==PIXY_RESULT_BUSY)
      {
        if(!wait)
          return PIXY_RESULT_BUSY; // new data not available yet
	}
    else if ((int8_t)m_buf[0]!=PIXY_RESULT_PROG_CHANGING)
        return m_buf[0];
    }
  }
  
  return PIXY_RESULT_ERROR;  // some kind of bitstream error
  
}

