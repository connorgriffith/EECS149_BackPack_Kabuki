/*
  Header file for DWM API
*/

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


/*
 * Initializes NRF spi device and configuration.
 * Parameters: 
 *    spi_instance: pointer to spi device instance for DWM
 *    spi_config:   pointer to spi device config for DWM
 */
void spi_init(nrf_drv_spi_t* spi_instance, nrf_drv_spi_config_t* spi_config);


/*
 * Switches 4 bytes from one endianness to another.
 * Parameters:
 *    start_int: pointer to the base byte of the int
 */
void switch_endianness(uint8_t* start_int);

/*
 * Performs the transfer of tx_buffer to the spi_instance and 
 *  fills rx_buffer with the response.
 * Parameters: 
 *    spi_instance: pointer to target device instance (DWM)
 *    tx_buffer:    pointer to transmit buffer
 *    tx_length:    length of transmit buffer
 *    rx_buffer:    pointer to empty receive buffer
 */
void spi_transfer(nrf_drv_spi_t* spi_instance, uint8_t* tx_buffer, uint8_t tx_length, uint8_t* rx_buffer);

/*
 * Resets DWM to idle mode
 * Parameters:
 *    spi_instance: pointer to target device instance (DWM)
 *    rx_buf:       pointer to empty receive buffer (DO WE NEED THIS, AMIRA??)
 *
 */
void nrf_spi_reset(nrf_drv_spi_t* spi_instance);

/*-----------------------DWM COMMANDS---------------------------*/

void factory_reset(nrf_drv_spi_t* spi_instance);

// THIS COMMAND DOESN'T WORK
void tag_cfg(nrf_drv_spi_t* spi_instance, uint8_t cfg_byte);

void get_node_id(nrf_drv_spi_t* spi_instance, uint64_t* id_buf);

void reboot_node(nrf_drv_spi_t* spi_instance);

// Gets location once and prints all returned information
void get_loc_single(nrf_drv_spi_t* spi_instance);

int32_t get_dist(nrf_drv_spi_t* spi_instance);

float get_dist_m(nrf_drv_spi_t* spi_instance);

// Might not work
void gpio_cfg_output(nrf_drv_spi_t* spi_instance, uint8_t pin, uint8_t signal);