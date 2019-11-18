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

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// global variables
KobukiSensors_t sensors = {0};

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
    }
}

const float CONVERSION = 0.00008529;
   
static float measure_distance(uint16_t current_encoder, uint16_t previous_encoder) {
    float difference = current_encoder - previous_encoder;
    return difference * CONVERSION;
}


uint16_t start_distance_encoder = 0;

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

  uint16_t start_distance_encoder = 0;
  int initialAngle = 0;

  states state = OFF;

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
          //mpu9250_start_gyro_integration();
           start_distance_encoder = sensors.leftWheelEncoder;

        } else {
          state = OFF;
          // perform state-specific actions here
          kobukiDriveDirect(0, 0);
        }
        break; // each case needs to end with break!
      }
      case DRIVING: {
        print_state(state);
        float traveled = measure_distance(sensors.leftWheelEncoder, start_distance_encoder);

        // transition logic
        if (is_button_pressed(&sensors)) {
          mpu9250_stop_gyro_integration();
          state = OFF;
        } else if (traveled > 1.0) {
          simple_ble_adv_only_name();

          state = TURNING;
          start_distance_encoder = sensors.leftWheelEncoder;
          // nrf_delay_ms(500);
          mpu9250_start_gyro_integration(); 

       } else {
          state = DRIVING;
          // perform state-specific actions here
          //uint16_t encoder = sensors.leftWheelEncoder;
          //int angle = (int) mpu9250_read_gyro_integration().z_axis;

          // if (angle > 360 || angle < 360) {
          //   angle = 0;
          // }
          
          char buf1[16];
          snprintf(buf1, 16, "%f m\n", traveled);
          printf(buf1);
          display_write(buf1, DISPLAY_LINE_1);

          // char buf[16];
          // snprintf(buf, 16, "%d", angle);
          // display_write(buf, DISPLAY_LINE_1);

          // simple_ble_adv_manuf_data((uint8_t*) &angle, sizeof(angle));

          kobukiDriveDirect(100, 100);
          advertising_stop();

        }
        break; // each case needs to end with break!
      }

      case TURNING: {
        print_state(state);
        int angle = (int) mpu9250_read_gyro_integration().z_axis;
        
        
        if (is_button_pressed(&sensors)) {
            mpu9250_stop_gyro_integration();
            state = OFF;
        }
 
        else if (abs(angle - initialAngle) >= 90) {
            kobukiDriveDirect(0, 0);
            // if (angle >= 350) 
            // {
            //   mpu9250_stop_gyro_integration();
            //   initialAngle = 0;
            // } else {

            //   initialAngle = angle;
            // }

            initialAngle = angle;
           
            // nrf_delay_ms(500);
            // mpu9250_stop_gyro_integration();
            start_distance_encoder = sensors.leftWheelEncoder;
           
            // nrf_delay_ms(500);
            state = DRIVING;


//Question why did he cast to an unsigned pointer when its is signed data?
            // simple_ble_adv_manuf_data((uint8_t*) &angle, sizeof(angle));
            simple_ble_adv_manuf_data((uint8_t*) &angle, 4);
            //update_gyro = true;

        } else {
            //int angle1 = (int) mpu9250_read_gyro_integration().z_axis;

            kobukiDriveDirect(50, -50);
            display_write("TURNING", DISPLAY_LINE_0);
            state = TURNING;
            //int turned = abs(angle);
            char buf[16];
            snprintf(buf, 16, "%d", angle);
            //printf(amount);
            display_write(buf, DISPLAY_LINE_1);
            // simple_ble_adv_manuf_data((uint8_t*) &angle, sizeof(angle));
        }
 
        break;
      }
    }
  }
}