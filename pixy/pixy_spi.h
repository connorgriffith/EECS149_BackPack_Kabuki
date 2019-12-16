  
#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"
#include "string.h"
#include "buckler.h"

static const nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
static nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;


#define SPI_MISO NRF_GPIO_PIN_MAP(0,27);
#define SPI_MOSI NRF_GPIO_PIN_MAP(0,26);
#define SPI_SCK NRF_GPIO_PIN_MAP(0,2);
#define SPI_SS NRF_GPIO_PIN_MAP(0,25);

#define PIXY_SPI_MAX_BUF_SIZE 0xFF
uint8_t pixy_spi_buf[PIXY_SPI_MAX_BUF_SIZE];

int8_t pixy_spi_open(uint32_t arg) {

    spi_config.sck_pin    = SPI_SCK;
    spi_config.miso_pin   = SPI_MISO;
    spi_config.mosi_pin   = SPI_MOSI;
    spi_config.ss_pin     = SPI_SS;
    spi_config.frequency  = NRF_DRV_SPI_FREQ_2M;
    spi_config.mode       = NRF_DRV_SPI_MODE_3;
    spi_config.bit_order  = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

    nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);

    return 0;
}

void pixy_spi_close() {
	nrf_drv_spi_uninit(&spi_instance);
}

int16_t pixy_spi_recv(uint8_t *data, uint8_t len, uint16_t *checksumCalculation) {

    if (checksumCalculation) {
      *checksumCalculation = 0;
  }

  nrf_drv_spi_transfer(&spi_instance, NULL, 0, pixy_spi_buf, len);

  for (int i = 0; i < len; i++) {
   data[i] = pixy_spi_buf[i];
   if (checksumCalculation) {
      *checksumCalculation += pixy_spi_buf[i];
  }
}

return len;
}

int16_t pixy_spi_send(uint8_t *data, uint8_t len) {

    for (int i = 0; i < len; i++) {
    	pixy_spi_buf[i] = data[i];
    }

    nrf_drv_spi_transfer(&spi_instance, pixy_spi_buf, len, NULL, 0);

    return len;
}

void pixy_spi_delayms(uint32_t ms) {
	nrf_delay_ms(ms);
}

void pixy_spi_delayus(uint32_t us) {
	nrf_delay_us(us);
}