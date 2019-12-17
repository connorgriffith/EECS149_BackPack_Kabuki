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

void spi_init(nrf_drv_spi_t* spi_instance, nrf_drv_spi_config_t* spi_config) {
  spi_config->sck_pin = DWM_SCLK;
  spi_config->mosi_pin = DWM_MOSI;
  spi_config->miso_pin = DWM_MISO;
  spi_config->ss_pin = DWM_CS;
  spi_config->irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
  spi_config->orc = 0;
  spi_config->frequency = NRF_DRV_SPI_FREQ_4M;
  spi_config->mode = NRF_DRV_SPI_MODE_0;
  spi_config->bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;

  APP_ERROR_CHECK(nrf_drv_spi_init(spi_instance, spi_config, NULL, NULL));
}

void switch_endianness(uint8_t* start_int) {
  uint8_t switched_int[4];
  for(int i = 0; i < 4; i++) {
    switched_int[i] = start_int[3-i];
  }  

  for(int k = 0; k < 4; k++) {
    start_int[k] = switched_int[k];
  }
}

void spi_transfer(nrf_drv_spi_t* spi_instance, uint8_t* tx_buffer, uint8_t tx_length, uint8_t* rx_buffer) {
  APP_ERROR_CHECK(nrf_drv_spi_transfer(spi_instance, tx_buffer, tx_length, rx_buffer, 2));
  printf("First poll rx_buffer: %x %x\n", rx_buffer[0], rx_buffer[1]);

  while(rx_buffer[0] == 0x00 || rx_buffer[0] == 0xff) { // && rx_buffer[1] == 0x00) {
    APP_ERROR_CHECK(nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buffer, 2));
    printf("Polling rx_buffer: %x %x\n", rx_buffer[0], rx_buffer[1]);
  }

  //printf("Last polled rx_buffer: %x %x\n", rx_buffer[0], rx_buffer[1]);
  uint8_t rx_buffer_length = rx_buffer[0];
  printf("SIZE: %d\n", rx_buffer_length);
  APP_ERROR_CHECK(nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buffer, rx_buffer_length));

  
  for(int k = 0; k < rx_buffer_length; k++) {
    printf("0x%x ", rx_buffer[k]);
  }

  printf("\n");
  printf("rx_buffer: %x %x %x\n", rx_buffer[0], rx_buffer[1], rx_buffer[2]);
  

  while(rx_buffer[0] == 0x00 || rx_buffer[0] == 0xff) {
    APP_ERROR_CHECK(nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buffer, rx_buffer_length));
    /*
    printf("rx_buffer: ");
    for(int k = 0; k < rx_buffer_length; k++) {
      printf("0x%x ", rx_buffer[k]);
    }
    printf("\n");
    */
  }

  //The two lines below are for printing the 4-byte distance from get_loc
  //to_little_endian((uint8_t*) (rx_buffer + 23));
  //printf("4-byte distance: 0x%x 0x%x 0x%x 0x%x\n\n", rx_buffer[23], rx_buffer[24], rx_buffer[25], rx_buffer[26]);

  printf("\n\n");
}

void nrf_spi_reset(nrf_drv_spi_t* spi_instance) {
  uint8_t reset_byte[1];
  reset_byte[0] = 0xff;
  for(int i = 0; i < 3; i++) {
    APP_ERROR_CHECK(nrf_drv_spi_transfer(spi_instance, reset_byte, 1, NULL, 0));
  }

  uint8_t rx_buf[3] = {0};
  while(rx_buf[0] != 0xff && rx_buf[1] != 0xff) {
    APP_ERROR_CHECK(nrf_drv_spi_transfer(spi_instance, NULL, 0, rx_buf, 2));
    nrf_delay_ms(100);
    //printf("rx_buf in reset: 0x%x 0x%x\n", rx_buf[0], rx_buf[1]);
  }

  uint8_t dummy_cmd[2] = {0};
  uint8_t dummy_rxbuf[3];
  spi_transfer(spi_instance, dummy_cmd, 2, dummy_rxbuf);
}

void factory_reset(nrf_drv_spi_t* spi_instance) {
  printf("Factory reset\n");
  uint8_t factory_rst_tx[2];
  factory_rst_tx[0] = 0x13;
  factory_rst_tx[1] = 0x00;

  uint8_t factory_rst_rx[3];
  spi_transfer(spi_instance, factory_rst_tx, 2, factory_rst_rx);
}

void tag_cfg(nrf_drv_spi_t* spi_instance, uint8_t cfg_byte) {
  printf("Configuring tag...\n");
  uint8_t tag_cfg_tx[4];
  tag_cfg_tx[0] = 0x05;
  tag_cfg_tx[1] = 0x02;
  tag_cfg_tx[2] = cfg_byte;
  tag_cfg_tx[3] = 0x00;

  //printf("Configure buffer: 0x%x 0x%x 0x%x 0x%x\n", tag_cfg_tx[0], tag_cfg_tx[1], tag_cfg_tx[2], tag_cfg_tx[3]);

  uint8_t tag_cfg_rx[3];
  spi_transfer(spi_instance, tag_cfg_tx, 4, tag_cfg_rx);
}

void get_node_id(nrf_drv_spi_t* spi_instance, uint64_t* id_buf) {
  printf("Getting node ID\n");
  uint8_t get_id_tx[2];
  get_id_tx[0] = 0x30;
  get_id_tx[1] = 0x00;

  uint8_t get_id_rx[13];
  nrf_delay_ms(50);
  spi_transfer(spi_instance, get_id_tx, 2, get_id_rx);
  memcpy(id_buf, (get_id_rx + 5), sizeof(*id_buf));
}

void reboot_node(nrf_drv_spi_t* spi_instance) {
  printf("Rebooting module\n");
  uint8_t reboot_tx[2];
  reboot_tx[0] = 0x14;
  reboot_tx[1] = 0x00;

  uint8_t reboot_rx[3];
  spi_transfer(spi_instance, reboot_tx, 2, reboot_rx);
}

void get_loc_single(nrf_drv_spi_t* spi_instance){
  printf("Getting tag location\n");
  uint8_t get_loc_tx[2];
  get_loc_tx[0] = 0x0C;
  get_loc_tx[1] = 0x00;

  uint8_t get_loc_rx[255];
  nrf_delay_ms(50);
  spi_transfer(spi_instance, get_loc_tx, 2, get_loc_rx); 
 
  nrf_delay_ms(50);
  printf("\n\nfinished transfer...parsing and printing:\n\n");
  printf("\tReturn value response:\t0x%x 0x%x 0x%x\n\n", get_loc_rx[0], get_loc_rx[1], get_loc_rx[2]);

  int32_t x_pos, y_pos, z_pos;
  memcpy(&x_pos, (get_loc_rx + 5), sizeof(x_pos));
  memcpy(&y_pos, (get_loc_rx + 9), sizeof(y_pos));
  memcpy(&z_pos, (get_loc_rx + 13), sizeof(z_pos));
  printf("\tTag position:\n\t\tType: 0x%x,\tLength: %d\n", get_loc_rx[3], get_loc_rx[4]);
  printf("\t\t\tx: %ld, y: %ld, z: %ld\n", x_pos, y_pos, z_pos);
  printf("\t\tTag position quality factor: %d\n\n", get_loc_rx[17]);

  int32_t distance;
  memcpy(&distance, get_loc_rx+23, sizeof(distance));
  memcpy(&x_pos, (get_loc_rx+28), sizeof(x_pos));
  memcpy(&y_pos, (get_loc_rx+32), sizeof(y_pos));
  memcpy(&z_pos, (get_loc_rx+36), sizeof(z_pos));
  printf("\tAnchor ranging:\n\t\tType: 0x%x,\tLength: %d\n", get_loc_rx[13], get_loc_rx[19]);
  printf("\t\tNumber of anchors: %d\n", get_loc_rx[20]);
  printf("\t\tAnchor #1 address: %x%x\n", get_loc_rx[21], get_loc_rx[22]);
  printf("\t\t4-byte distance: 0x%x 0x%x 0x%x 0x%x\n", get_loc_rx[23], get_loc_rx[24], get_loc_rx[25], get_loc_rx[26]);
  printf("\t\tDistance to anchor #1: %ld\n", distance);
  printf("\t\tDistance quality factor: %d\n", get_loc_rx[27]);
  printf("\t\tAnchor position:\n");
  printf("\t\t\tx: %ld, y: %ld, z: %ld\n", x_pos, y_pos, z_pos);
  printf("\t\tAnchor position quality factor: %d\n\n", get_loc_rx[40]);
}

int32_t get_dist(nrf_drv_spi_t* spi_instance) {
  printf("Getting distance\n");
  uint8_t get_loc_tx[2];
  get_loc_tx[0] = 0x0C;
  get_loc_tx[1] = 0x00;

  uint8_t get_loc_rx[255];
  int32_t avg_dist = 0;
  int32_t curr_dist;
  spi_transfer(spi_instance, get_loc_tx, 2, get_loc_rx);
  nrf_delay_ms(50);
  spi_transfer(spi_instance, get_loc_tx, 2, get_loc_rx);

  for(int i = 0; i < 5; i++) {
    nrf_delay_ms(50);
    spi_transfer(spi_instance, get_loc_tx, 2, get_loc_rx);
    memcpy(&curr_dist, (get_loc_rx + 23), sizeof(curr_dist));
    avg_dist += curr_dist;
  }

  return (avg_dist / 5);
}

void gpio_cfg_output(nrf_drv_spi_t* spi_instance, uint8_t pin, uint8_t signal) {
  printf("Configuring output pin\n");
  uint8_t tx_buf[4];
  tx_buf[0] = 0x28;
  tx_buf[1] = 0x02;
  tx_buf[2] = pin;
  tx_buf[3] = signal;

  uint8_t rx_buf[3];
  rx_buf[0] = 0;
  rx_buf[1] = 0;
  rx_buf[2] = 0;
  spi_transfer(spi_instance, tx_buf, 4, rx_buf);
}
