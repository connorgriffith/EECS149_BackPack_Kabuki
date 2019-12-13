/**
 * Copyright (c) 2017 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "nrfx.h"
#include "nrfx_spi.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
#include <stdlib.h>
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
/*
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
*/

#define NRFX_SPI_SCK   NRF_GPIO_PIN_MAP(0, 17)
#define NRFX_SPI_MOSI  NRF_GPIO_PIN_MAP(0, 15)
#define NRFX_SPI_MISO  NRF_GPIO_PIN_MAP(0, 16)
#define NRFX_SPI_SS    NRF_GPIO_PIN_MAP(0, 18)

#define SPI_INSTANCE 1
#define TLV_TYPE_CFG_TN_SET 5

/*
#define NRFX_SPIM_SCK_PIN  3
#define NRFX_SPIM_MOSI_PIN 4
#define NRFX_SPIM_MISO_PIN 28
#define NRFX_SPIM_SS_PIN   29
#define NRFX_SPIM_DCX_PIN  30
*/

//#define SPI_INSTANCE  3                                           /**< SPI instance index. */

static const nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
static nrfx_spi_t spi; // = spi_instance.u.spi;  /**< SPI instance. */
static nrfx_spi_config_t spi_config = NRFX_SPI_DEFAULT_CONFIG;

//#define TEST_STRING "Nordic123456789012345678901234567890"
//static uint8_t       m_tx_buf[] = TEST_STRING;           /**< TX buffer. */
//static uint8_t       m_rx_buf[sizeof(TEST_STRING) + 1];  /**< RX buffer. */
//static const uint8_t m_length = sizeof(m_tx_buf);        /**< Transfer length. */

void init(void) {
  spi = spi_instance.u.spi;

  spi_config.frequency = NRF_SPI_FREQ_4M;
  spi_config.ss_pin = NRFX_SPI_SS;
  spi_config.sck_pin = NRFX_SPI_SCK; 
  spi_config.mosi_pin = NRFX_SPI_MOSI;
  spi_config.miso_pin = NRFX_SPI_MISO;
  spi_config.mode = NRF_SPI_MODE_1;

  ret_code_t err_code = nrfx_spi_init(&spi, &spi_config, NULL, NULL);
  APP_ERROR_CHECK(err_code);
  if (err_code != NRF_SUCCESS) {
    return;
  }
}

int main(void)
{
  init();

  uint8_t cfg_tx_buffer[4];
  cfg_tx_buffer[0] = TLV_TYPE_CFG_TN_SET;
  cfg_tx_buffer[1] = 2;
  cfg_tx_buffer[2] = 0x62;
  cfg_tx_buffer[3] = 0; 

  uint8_t cfg_rx_buffer[3];
  nrfx_spi_xfer_desc_t cfg_desc = NRFX_SPI_XFER_TRX(cfg_tx_buffer, 4, cfg_rx_buffer, 3);
  ret_code_t err_code = nrfx_spi_xfer(&spi, &cfg_desc, 0);
  APP_ERROR_CHECK(err_code);
  if (err_code != NRF_SUCCESS) {
    return 0;
  }
  while (cfg_rx_buffer[0] == 0x00) {
    printf("Receiving 0s\n");
    nrf_delay_ms(10);
    nrfx_spi_xfer(&spi, &cfg_desc, 0);
  } 
}
