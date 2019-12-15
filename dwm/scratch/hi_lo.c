// Robot Template app
//
// Framework for creating applications that control the Kobuki robot

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_spi.h"

#include "buckler.h"

#define DWM_CS   NRF_GPIO_PIN_MAP(0,18)
#define DWM_SCLK NRF_GPIO_PIN_MAP(0,17)
#define DWM_MOSI NRF_GPIO_PIN_MAP(0,16)
#define DWM_MISO NRF_GPIO_PIN_MAP(0,15)

nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
//nrf_drv_spi_config_t spi_config;
/*
void init(void) {
  spi_config.sck_pin      = DWM_SCLK;
  spi_config.mosi_pin     = DWM_MOSI;
  spi_config.miso_pin     = DWM_MISO;
  spi_config.ss_pin       = DWM_CS;
  spi_config.irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
  spi_config.orc          = 0;
  spi_config.frequency    = NRF_DRV_SPI_FREQ_4M;
  spi_config.mode         = NRF_DRV_SPI_MODE_0;
  spi_config.bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
  
  APP_ERROR_CHECK(nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL));
}
*/

int main(void) {
    ret_code_t error_code = NRF_SUCCESS;

    // initialize RTT library
    error_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(error_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    printf("Log initialized!\n");

    // initialize display
    nrf_drv_spi_config_t spi_config = {
        .sck_pin = DWM_SCLK,
        .mosi_pin = DWM_MOSI,
        .miso_pin = DWM_MISO,
        .ss_pin = DWM_CS,
        .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
        .orc = 0,
        .frequency = NRF_DRV_SPI_FREQ_4M,
        .mode = NRF_DRV_SPI_MODE_0,
        .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
    };
    error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
    APP_ERROR_CHECK(error_code);

    uint8_t reset_buf[1];
    reset_buf[0] = 0xff;

    uint8_t tx_buf[4];
    tx_buf[0] = 0x28;
    tx_buf[1] = 0x02;
    tx_buf[2] = 0x0D;
    tx_buf[3] = 0x01;

    uint8_t rx_buf[3];
    rx_buf[0] = 0;
    rx_buf[1] = 0;
    rx_buf[2] = 0;

    ret_code_t err_code = nrf_drv_spi_transfer(&spi_instance, reset_buf, 1, NULL, 0);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_spi_transfer(&spi_instance, reset_buf, 1, NULL, 0);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_spi_transfer(&spi_instance, reset_buf, 1, NULL, 0);
    APP_ERROR_CHECK(err_code);

    printf("reset\n");
    //while (rx_buf[0] == 0 && rx_buf[1] == 0) {
    while(rx_buf[0] != 0xff && rx_buf[2] != 0xff) {
        err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 3);
        APP_ERROR_CHECK(err_code);
        if (err_code != NRF_SUCCESS) {
            printf("continuing spi error code: %d\n", (int) err_code);
        }
        else {
            printf("rx_buf: %x %x %x\n", rx_buf[0], rx_buf[1], rx_buf[2]);
        }
        nrf_delay_ms(100);
    }

    nrf_delay_ms(1000);
    printf("Trying to send command\n");
    err_code = nrf_drv_spi_transfer(&spi_instance, tx_buf, 4, NULL, 0);
    APP_ERROR_CHECK(err_code);
    //err_code = nrf_drv_spi_transfer(&spi_instance, tx_buf, 4, rx_buf, 2);
    err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 3);
    APP_ERROR_CHECK(err_code);
    if (err_code != NRF_SUCCESS) {
        printf("spi error code: %d\n", (int) err_code);
    }

    while (rx_buf[0] == 0 && rx_buf[1] == 0) {
        err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 3);
        APP_ERROR_CHECK(err_code);
        if (err_code != NRF_SUCCESS) {
            printf("continuing spi error code: %d\n", (int) err_code);
        }
        else {
            printf("rx_buf after reading 3: %x %x %x\n", rx_buf[0], rx_buf[1], rx_buf[2]);
        }
    }

    printf("received: %x %x\n", rx_buf[0], rx_buf[1]);

    while (rx_buf[0] == 0 || rx_buf[0] == 0xff) {
        err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 3);
        APP_ERROR_CHECK(err_code);
        if (err_code != NRF_SUCCESS) {
            printf("continuing spi error code: %d\n", (int) err_code);
        }
        else {
            printf("rx_buf (this does run): %x %x %x\n", rx_buf[0], rx_buf[1], rx_buf[2]);
        }
    }

    // printf("received: %x %x\n", rx_buf[0], rx_buf[1]);
    printf("\nSending low command..\n");
    tx_buf[3] = 0;
    nrf_delay_ms(1000);
    err_code = nrf_drv_spi_transfer(&spi_instance, tx_buf, 4, NULL, 0);
    APP_ERROR_CHECK(err_code);
    //err_code = nrf_drv_spi_transfer(&spi_instance, tx_buf, 4, rx_buf, 2);
    err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 3);
    APP_ERROR_CHECK(err_code);
    if (err_code != NRF_SUCCESS) {
        printf("spi error code: %d\n", (int) err_code);
    }

    //while (rx_buf[0] == 0 && rx_buf[1] == 0) {
    //while((rx_buf[0] == 0xff && rx_buf[1] == 0xff) || (rx_buf[0] == 0 && rx_buf[1] == 0)) {
    while(rx_buf[0] == 0 || rx_buf[0] == 0xff) {
        err_code = nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 2);
        APP_ERROR_CHECK(err_code);
        if (err_code != NRF_SUCCESS) {
            printf("continuing spi error code: %d\n", (int) err_code);
        }
        else {
            printf("rx_buf polling 2: %x %x\n", rx_buf[0], rx_buf[1]);
        }
    }

    while(rx_buf[0] != 0x40) {
      nrf_drv_spi_transfer(&spi_instance, NULL, 0, rx_buf, 3);
      printf("rx_buf: %x %x %x\n", rx_buf[0], rx_buf[1], rx_buf[2]);
    }

    printf("received: %x %x\n", rx_buf[0], rx_buf[1]);


    // loop forever, running state machine
    while (1) {
        nrf_delay_ms(1);
    }
}

