// BLE advertisement template
//
// Advertises device name: EE149

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_util.h"
#include "nrf_twi_mngr.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "buckler.h"
#include "simple_ble.h"

#include "max44009.h"
#include "mpu9250.h"

#include "app_error.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_spi.h"

#include "display.h"
#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"


// Create a timer
APP_TIMER_DEF(adv_timer);

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:49:xx:xx
        .platform_id       = 0x49,    // used as 4th octect in device BLE address
        .device_id         = 0xFFFF,  // TODO: replace with your lab bench number
        .adv_name          = "EE149", // Note that this name is not displayed to save room in the advertisement for data.
        .adv_interval      = MSEC_TO_UNITS(100, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(50, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(80, UNIT_1_25_MS),
};

bool update_gyro = false;

void gyro_timer_callback() {
    printf("gyro timer fired!\n");
    // TODO: implement this function!
    // Use Simple BLE function to read light sensor and put data in
    // advertisement, but be careful about doing it in the callback itself!
    update_gyro = true;
}

/*******************************************************************************
 *   State for this application
 ******************************************************************************/
// Main application state
simple_ble_app_t* simple_ble_app;

  //Connor Added
  void start_gyro() {
    mpu9250_start_gyro_integration();
  }

  void stop_gyro() {
    mpu9250_stop_gyro_integration();
  }

  float read_gyro() {
    return mpu9250_read_gyro_integration().z_axis;
  }



int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  // Initialize

  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized\n");

  // initialize i2c master (two wire interface)
  // nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  // i2c_config.scl = BUCKLER_SENSORS_SCL;
  // i2c_config.sda = BUCKLER_SENSORS_SDA;
  // i2c_config.frequency = NRF_TWIM_FREQ_100K;
  // error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
  // APP_ERROR_CHECK(error_code);


  // error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  // APP_ERROR_CHECK(error_code);

  // initialize display
  nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
  nrf_drv_spi_config_t spi_config = {
    .sck_pin = BUCKLER_LCD_SCLK,
    .mosi_pin = BUCKLER_LCD_MOSI,
    .miso_pin = BUCKLER_LCD_MISO,
    .ss_pin = BUCKLER_LCD_CS,
    .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
    .orc = 0,
    .frequency = NRF_DRV_SPI_FREQ_4M,
    .mode = NRF_DRV_SPI_MODE_2,
    .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
  };
  error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);
  display_init(&spi_instance);
  printf("Display initialized!\n");


  // initialize i2c master (two wire interface)
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = BUCKLER_SENSORS_SCL;
  i2c_config.sda = BUCKLER_SENSORS_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
  APP_ERROR_CHECK(error_code);
  mpu9250_init(&twi_mngr_instance);
  printf("IMU initialized!\n");

   // initialize Kobuki
  kobukiInit();
  printf("Kobuki initialized!\n");



  // initialize MAX44009 driver
  // const max44009_config_t config = {
  //   .continuous = 0,
  //   .manual = 0,
  //   .cdr = 0,
  //   .int_time = 3,
  // };
  // max44009_init(&twi_mngr_instance, BUCKLER_LIGHT_INTERRUPT);
  // max44009_config(config);
  // printf("MAX44009 initialized\n");

  // Setup BLE
  simple_ble_app = simple_ble_init(&ble_config);

  // TODO replace this with advertisement sending light data
  //simple_ble_adv_only_name();
  //uint8_t test_num = 255;
  //uint8_t* test_buf = &test_num; 
  //simple_ble_adv_manuf_data(test_buf, 1);

  // Set a timer to read the light sensor and update advertisement data 0.1 seconds.
  app_timer_init();
  app_timer_create(&adv_timer, APP_TIMER_MODE_REPEATED, (app_timer_timeout_handler_t) gyro_timer_callback);
  app_timer_start(adv_timer, APP_TIMER_TICKS(100), NULL); // 100 milliseconds


  start_gyro();

  while(1) {
    // Sleep while SoftDevice handles BLE
    power_manage();

    if (update_gyro) {
      float gyro_val = read_gyro();
      simple_ble_adv_manuf_data((uint8_t *) &gyro_val, 4);
      //update_lux = false;
    }
  }
}

