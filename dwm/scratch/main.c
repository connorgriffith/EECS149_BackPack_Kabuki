#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
//#include "dwm_api.h"
//#include "dwm1001_tlv.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <stdlib.h>


#define SPI_BITS                 8
#define SPI_SPEED                8000000
#define SPI_DELAY                0

/*
#define SPI_SCLK    NRF_GPIO_PIN_MAP(0,13)
#define SPI_MISO    NRF_GPIO_PIN_MAP(0,12)
#define SPI_MOSI    NRF_GPIO_PIN_MAP(0,11)
*/

#define SPI_SCLK BUCKLER_LCD_SCLK
#define SPI_MISO BUCKLER_LCD_MISO
#define SPI_MOSI BUCKLER_LCD_MOSI

#define TLV_MAX_SIZE 255
#define TLV_TYPE_CFG_TN_SET 5

static const nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
static nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

int init(void) {
  spi_config.sck_pin    = SPI_SCLK;
  spi_config.miso_pin   = SPI_MISO;
  spi_config.mosi_pin   = SPI_MOSI;
  // spi_config.ss_pin     = RTC_CS;
  spi_config.frequency  = SPI_SPEED;
  spi_config.mode       = NRF_DRV_SPI_MODE_0;
  spi_config.bit_order  = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST; 
  return nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
}

void shift_bytes(uint8_t* buffer, uint8_t length) {
  for (int i = 0; i < length; i++) {
    buffer[i] = buffer[i] << 1;
  }
}

/* Poll for 2
   Check
   Get NOT 255
   Store those 2 (received_bytes)
   call transfer(..,..,.., rx_buffer, received_bytes[1])  
*/
uint8_t spi_transfer(uint8_t* tx_buffer, uint8_t* rx_buffer, uint8_t tx_length) {
  nrf_drv_spi_transfer(&spi_instance, tx_buffer, 2, rx_buffer, 2); 
  while (rx_buffer[0] == 0x00) { // || rx_buffer[1] == 0x00 ) {//|| rx_buffer[0] == 0xff || rx_buffer[1] == 0xff) {
    nrf_delay_ms(10);
    printf("Polling 2 bytes...\n");
    nrf_drv_spi_transfer(&spi_instance, tx_buffer, 2, rx_buffer, 2);
  }
  
  uint8_t rx_buffer_length = rx_buffer[1];

  printf("Type:\t%x\n", rx_buffer[0]);
  printf("Length:\t%d\n\n", rx_buffer_length);
 
  nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buffer + 2, rx_buffer_length);

  for(int i = 0; i < tx_length; i++) {
    printf("TX_buffer[%d]:\t%x\n", i, tx_buffer[i]);
  }

  printf("\n");

  for(int k = 0; k < rx_buffer_length + 2; k++) {
    printf("RX_buffer[%d]:\t%x\n", k, rx_buffer[k]);
  }
  
  return rx_buffer_length;
}

int main(void) { 
  init();  
/*
  uint8_t data[2];
  data[0] = 0x14;
  data[1] = 0x00;
  shift_bytes(data, 2);
  ret_code_t err_code = nrf_drv_spi_transfer(&spi_instance, data, 2, NULL, 0);
  APP_ERROR_CHECK(err_code);
  if (err_code != NRF_SUCCESS) {
    return false;  
  }
  uint8_t size_num[2];
  err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, size_num, 2);
  while (size_num[0] == 0x00) {
    APP_ERROR_CHECK(err_code);
    if (err_code != NRF_SUCCESS) {
      return false;
    }
    nrf_delay_ms(10);
    err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, size_num, 2);
  }
  printf("%x %x\n", size_num[0], size_num[1]);
  uint8_t* readData = (uint8_t *)malloc(sizeof(uint8_t)*size_num[0]);
  err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, readData, size_num[0]);
  APP_ERROR_CHECK(err_code);
  if (err_code != NRF_SUCCESS) {
    return false;
  }
*/
  uint8_t reset_request[2];
  reset_request[0] = 0x14;
  reset_request[1] = 0;
  shift_bytes(reset_request, 2);  

  uint8_t reset_response[3];
  reset_response[0] = 0x00;
  spi_transfer(reset_request, reset_response, 2);
 
  uint8_t cfg_tx_buffer[4];
  cfg_tx_buffer[0] = TLV_TYPE_CFG_TN_SET;
  cfg_tx_buffer[1] = 2;
  cfg_tx_buffer[2] = 0x62;
  cfg_tx_buffer[3] = 0; 
  shift_bytes(cfg_tx_buffer, 4);

  uint8_t cfg_rx_buffer[TLV_MAX_SIZE];
  cfg_rx_buffer[0] = 0x00;
  spi_transfer(cfg_rx_buffer, cfg_rx_buffer, 4);


}
