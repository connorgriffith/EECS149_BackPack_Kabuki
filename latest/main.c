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
#include "display.h"
#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"
#include "mpu9250.h"
#include "simple_ble.h"

#include "states.h"

const float CONVERSION = 0.00008529;

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// global variables
KobukiSensors_t sensors = {0};
uint16_t start_distance_encoder = 0;

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:49:xx:xx
        .platform_id       = 0x49,    // used as 4th octect in device BLE address
        .device_id         = 0x0000, // TODO: replace with your lab bench number
        .adv_name          = "KOBUKI", // used in advertisements if there is room
        .adv_interval      = MSEC_TO_UNITS(100, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
};

simple_ble_app_t* simple_ble_app;

void print_state(states current_state){
	switch(current_state){
  	case OFF:
  		display_write("OFF", DISPLAY_LINE_0);
  		break;
  	case DRIVING:
  		display_write("DRIVING", DISPLAY_LINE_0);
  		break;
    case TURNING:
      display_write("TURNING", DISPLAY_LINE_0);
      break;
    default:
      break;
  }
}
   
static float measure_distance(uint16_t current_encoder, uint16_t previous_encoder) {
    float difference = current_encoder - previous_encoder;
    return difference * CONVERSION;
}

int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized!\n");

  // Setup BLE
  simple_ble_app = simple_ble_init(&ble_config);

  // Start Advertising
  simple_ble_adv_only_name();

  // initialize LEDs
  nrf_gpio_pin_dir_set(23, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(24, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(25, NRF_GPIO_PIN_DIR_OUTPUT);

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

  states state = OFF;
  states next_state = OFF;
  float distance_traveled = 0.0;
  int16_t angle_turned = 0;
  uint16_t initial_encoder = 0;
  uint8_t brake_counter = 0;

  // loop forever, running state machine
  while (1) {
    // read sensors from robot
    kobukiSensorPoll(&sensors);

    // TODO: complete state machine
    switch(state) {
      case OFF: {
        print_state(state);

        // transition logic
        if (is_button_pressed(&sensors)) {
          state = DRIVING;
          initial_encoder = 0;
          distance_traveled = 0.0;
          // mpu9250_start_gyro_integration();
        } else {
          state = OFF;
          // perform state-specific actions here
          distance_traveled = 0.0;
          angle_turned = 0;
          brake_counter = 0;
          kobukiDriveDirect(0, 0);
        }
        break; // each case needs to end with break!
      }

      case DRIVING: {
        print_state(state);

        // transition logic
        if (is_button_pressed(&sensors)) {
          // mpu9250_stop_gyro_integration();
          state = OFF;
        } else if (distance_traveled >= 0.5) {
          state = BRAKE;
          next_state = TURNING;
          distance_traveled = 0.0;
        } else {
          state = DRIVING;
          kobukiDriveDirect(100,100);
          distance_traveled += measure_distance(sensors.leftWheelEncoder, initial_encoder);
          initial_encoder = sensors.leftWheelEncoder;
          char dist[16];
          snprintf(dist, 16, "%f", distance_traveled);
          display_write(dist, DISPLAY_LINE_1);
          // perform state-specific actions here
          // uint16_t encoder = sensors.leftWheelEncoder;
          // float angle = mpu9250_read_gyro_integration().z_axis;
          // simple_ble_adv_manuf_data((uint8_t*) &angle, sizeof(angle));
          // kobukiDriveDirect(0, 100);
        }
        break; // each case needs to end with break!
      }

      case TURNING: {
        print_state(state);

        if (is_button_pressed(&sensors)) {
          state = OFF;
          angle_turned = 0;
          mpu9250_stop_gyro_integration();
        } else if (abs(angle_turned) >= 90) {
          state = BRAKE;
          next_state = DRIVING;
          simple_ble_adv_manuf_data((uint8_t*) &angle_turned, sizeof(angle_turned));
          angle_turned = 0;
          mpu9250_stop_gyro_integration();
        } else {
          state = TURNING;
          kobukiDriveDirect(50, -50);
          angle_turned = (int) mpu9250_read_gyro_integration().z_axis;
          char buf[16];
          snprintf(buf, 16, "%d", angle_turned);
          display_write(buf, DISPLAY_LINE_1);
        }
        break;
      }

      case BRAKE: {
        if (is_button_pressed(&sensors)) {
          state = OFF;
        } else if (brake_counter < 5) {
          state = BRAKE;
          brake_counter++;
          kobukiDriveDirect(0,0);
        } else {
          brake_counter = 0;
          state = next_state;
          initial_encoder = sensors.leftWheelEncoder;
          if (state == TURNING) {
            mpu9250_start_gyro_integration();
          }
        }
        break;
      }

    }
  }
}