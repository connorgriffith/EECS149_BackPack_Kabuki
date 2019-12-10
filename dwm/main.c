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


#define SPI_BITS                 8
#define SPI_SPEED                8000000
#define SPI_DELAY                0

#define SPI_SCLK    NRF_GPIO_PIN_MAP(0,13)
#define SPI_MISO    NRF_GPIO_PIN_MAP(0,12)
#define SPI_MOSI    NRF_GPIO_PIN_MAP(0,11)

#define TLV_MAX_SIZE 255
#define TLV_TYPE_CFG_TN_SET 5

static const nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
static nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
static volatile bool transfer_done;

static uint8_t cfg_tx_buffer[TLV_MAX_SIZE];
static uint8_t cfg_rx_buffer[TLV_MAX_SIZE]; 

void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context) {
  transfer_done = cfg_rx_buffer[0] != 255;
}

int init(void) {
  spi_config.sck_pin    = SPI_SCLK;
  spi_config.miso_pin   = SPI_MISO;
  spi_config.mosi_pin   = SPI_MOSI;
  // spi_config.ss_pin     = RTC_CS;
  spi_config.frequency  = SPI_SPEED;
  spi_config.mode       = NRF_DRV_SPI_MODE_0;
  spi_config.bit_order  = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST; 
  return nrf_drv_spi_init(&spi_instance, &spi_config, spi_event_handler, NULL);
}


int main(void) { 
  init();
  
  uint8_t reset_request[2];
  reset_request[0] = 20;
  reset_request[1] = 0;
  
  uint8_t reset_response[3];
  nrf_drv_spi_transfer(&spi_instance, reset_request, 2, reset_response, 3); 
  
  for(int i = 0; i < 3; i++) {
    printf("reset_response[%d]:\t%d\n", i, reset_response[i]);
  }

  //uint8_t cfg_tx_buffer[TLV_MAX_SIZE];
  cfg_tx_buffer[0] = TLV_TYPE_CFG_TN_SET << 1;
  cfg_tx_buffer[1] = 2 << 1;
  cfg_tx_buffer[2] = 98 << 1;
  cfg_tx_buffer[3] = 0 << 1;
  
  //uint8_t cfg_rx_buffer[TLV_MAX_SIZE];

  transfer_done = false;
  nrf_drv_spi_transfer(&spi_instance, cfg_tx_buffer, 4, cfg_rx_buffer, 3);
  
  while(!transfer_done) {
    printf("In while loop...\n");
    __WFE();
  }

  printf("I'm not stuck :)\n");  

  for(int i = 0; i < 4; i++) {
    printf("TX_buffer[%d]:\t%d\n", i, cfg_tx_buffer[i]);
  }

  for(int k = 0; k < 3; k++) {
    printf("RX_buffer[%d]:\t%x\n", k, cfg_rx_buffer[k]);
  }
  
}
