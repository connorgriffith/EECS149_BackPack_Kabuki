// Robot Template app
//
// Framework for creating applications that control the Kobuki robot

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_dwm.h"

#define DWM_CS   NRF_GPIO_PIN_MAP(0,18)
#define DWM_SCLK NRF_GPIO_PIN_MAP(0,17)
#define DWM_MOSI NRF_GPIO_PIN_MAP(0,16)
#define DWM_MISO NRF_GPIO_PIN_MAP(0,15)

nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
nrf_drv_spi_config_t spi_config;

int main(void) {
  spi_init(&spi_instance, &spi_config);
  nrf_spi_reset(&spi_instance);
  reboot_node(&spi_instance);
  //get_loc_single(&spi_instance);
  //int32_t distance = get_dist(&spi_instance);
  //printf("Calculated Distance: %ld\n\n", distance); 
  while (1) {
      nrf_delay_ms(1);
  }
}

