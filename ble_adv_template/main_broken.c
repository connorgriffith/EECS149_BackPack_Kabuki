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

typedef enum {
    OFF=0,
    FORWARD,
    DRIVING,
    BACKWARD,
    TURNING,
    RIGHT,
    LEFT,
    OBSTACLE,
    OBSTACLE_LEFT,
    OBSTACLE_RIGHT
} states;

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:49:xx:xx
        .platform_id       = 0x49,    // used as 4th octect in device BLE address
        .device_id         = 0xAAAA,  // TODO: replace with your lab bench number
        .adv_name          = "EE149", // Note that this name is not displayed to save room in the advertisement for data.
        .adv_interval      = MSEC_TO_UNITS(100, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(50, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(80, UNIT_1_25_MS),
};

bool update_gyro = false;

void gyro_timer_callback() {
    //printf("gyro timer fired!\n");
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


	const float CONVERSION = 0.00008529;
	 
	static float measure_distance(uint16_t current_encoder, uint16_t previous_encoder) {
	    float difference = current_encoder - previous_encoder;
	    return difference * CONVERSION;
	}
	 
	static float reverse_measure_distance(uint16_t current_encoder, uint16_t previous_encoder) {
	    float difference = -current_encoder + previous_encoder;
	    return difference * CONVERSION;
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
  display_write("OH GOD", DISPLAY_LINE_0);
  simple_ble_app = simple_ble_init(&ble_config);
  display_write("HERE WE GO AGAIN", DISPLAY_LINE_0);

  // configure initial state
  states state = OFF;
  printf("state initialized\n");
  KobukiSensors_t sensors = {0};
  printf("sensors initialized\n");
  uint16_t start_distance_encoder = 0;
  printf("sde initialized\n");
  mpu9250_measurement_t angle;
  printf("angle initialized\n");
  char distance[15] = {0};
  printf("distance initialized\n");
  char amount[15] = {0};
  printf("amount initialized\n");
  int collision = 3;
  printf("collision initialized\n");
 

  // TODO replace this with advertisement sending light data
  //simple_ble_adv_only_name();
  //uint8_t test_num = 255;
  //uint8_t* test_buf = &test_num; 
  //simple_ble_adv_manuf_data(test_buf, 1);

  // Set a timer to read the light sensor and update advertisement data 0.1 seconds.
  // app_timer_init();
  // app_timer_create(&adv_timer, APP_TIMER_MODE_REPEATED, (app_timer_timeout_handler_t) gyro_timer_callback);
  // app_timer_start(adv_timer, APP_TIMER_TICKS(100), NULL); // 100 milliseconds


  // start_gyro();
 
 //int i = 0;
  // loop forever, running state machine
  while (1) {
    // read sensors from robot
    //i = i + 1;
    if (state != OFF) {
    	kobukiSensorPoll(&sensors);
 	}
    // delay before continuing
    // Note: removing this delay will make responses quicker, but will result
    //  in printf's in this loop breaking JTAG
    //printf("Delay\n");
    nrf_delay_ms(1);
 

    if (update_gyro) {
        // Setup BLE
      
      //simple_ble_app = simple_ble_init(&ble_config);
      
      int gyro_val = (int)read_gyro();
      char buf[16];
      snprintf(buf, 16, "%d", gyro_val);
      display_write(buf, DISPLAY_LINE_0);
      // printf("%f\n", gyro_val);
      simple_ble_adv_manuf_data((uint8_t *) &gyro_val, 4);
      advertising_stop();
      update_gyro = false;
	  mpu9250_stop_gyro_integration();
      //update_lux = false;
    }

    // handle states
    switch(state) {
      case OFF: {
        // transition logic
        if (is_button_pressed(&sensors)) {
            printf("button pressed\n");
            start_distance_encoder = sensors.leftWheelEncoder;
          state = DRIVING;
        } else {
          // perform state-specific actions here
          display_write("OFF", DISPLAY_LINE_0);
          printf("off\n");
          //kobukiDriveDirect(0, 0);
          state = OFF;
        }
        break; // each case needs to end with break!
      }
 
      case DRIVING: {
        // transition logic

        float traveled = measure_distance(sensors.leftWheelEncoder, start_distance_encoder);
          snprintf(distance, 15, "%f m\n", traveled);
          printf(distance);
          display_write(distance, DISPLAY_LINE_1);
        if (is_button_pressed(&sensors)) {
          state = OFF;
        }  else if (measure_distance(sensors.leftWheelEncoder, start_distance_encoder) > 1) {
          start_distance_encoder = sensors.leftWheelEncoder;
        } else if (measure_distance(sensors.leftWheelEncoder, start_distance_encoder) > 0.5) {
          state = TURNING;
          // nrf_delay_ms(500);
          mpu9250_start_gyro_integration();
        } else {
          // perform state-specific actions here
          display_write("DRIVING", DISPLAY_LINE_0);
         
          printf("driving\n");
          kobukiDriveDirect(100, 100);
          state = DRIVING;
        }
        break; // each case needs to end with break!
      }
     
      case TURNING: {
        angle = mpu9250_read_gyro_integration();
       
        if (is_button_pressed(&sensors)) {
            mpu9250_stop_gyro_integration();
            state = OFF;
        }
 
        else if (abs(angle.z_axis) >= 90) {
            kobukiDriveDirect(0, 0);
            display_write("I turned.", DISPLAY_LINE_0);
            // nrf_delay_ms(500);
            // mpu9250_stop_gyro_integration();
            start_distance_encoder = sensors.leftWheelEncoder;
           
            // nrf_delay_ms(500);
            state = DRIVING;
            update_gyro = true;

        } else {
            kobukiDriveDirect(50, -50);
            display_write("TURNING", DISPLAY_LINE_0);
            state = TURNING;
            float turned = abs(angle.z_axis);
            snprintf(amount, 15, "%f d\n", turned);
            printf(amount);
            display_write(amount, DISPLAY_LINE_1);
        }
 
        break;
      }
 
      // case OBSTACLE: {
      //   display_write("OBSTACLE", DISPLAY_LINE_0);
      //   display_write("", DISPLAY_LINE_1);
      //   if (is_button_pressed(&sensors)) {
      //     state = OFF;
      //   } else if (reverse_measure_distance(sensors.leftWheelEncoder, start_distance_encoder) > 0.5) {
      //     start_distance_encoder = sensors.leftWheelEncoder;
      //   } else if (reverse_measure_distance(sensors.leftWheelEncoder, start_distance_encoder) > 0.1) {
      //     mpu9250_start_gyro_integration();
      //     if (collision == 0) {
      //       state = OBSTACLE_LEFT;
      //     } else if (collision == 1) {
      //       state = OBSTACLE_RIGHT;
      //     } else {
      //       display_write("ERROR", DISPLAY_LINE_0);
      //     }
         
      //   }
      //   kobukiDriveDirect(-100, -100);
      //   break;
      // }
 
      // case OBSTACLE_LEFT: {
      //   angle = mpu9250_read_gyro_integration();
       
      //   if (is_button_pressed(&sensors)) {
      //       mpu9250_stop_gyro_integration();
      //       state = OFF;
      //   } else if (abs(angle.z_axis) >= 45) {
      //       kobukiDriveDirect(0, 0);
      //       // nrf_delay_ms(500);
      //       mpu9250_stop_gyro_integration();
      //       start_distance_encoder = sensors.leftWheelEncoder;
           
      //       // nrf_delay_ms(500);
      //       state = DRIVING;
      //   } else {
      //       kobukiDriveDirect(50, -50);
      //       display_write("TURNING RIGHT", DISPLAY_LINE_0);
      //       state = OBSTACLE_LEFT;
      //       float turned = abs(angle.z_axis);
      //       snprintf(amount, 15, "%f d\n", turned);
      //       printf(amount);
      //       display_write(amount, DISPLAY_LINE_1);
      //   }
      //   break;
      // }
 
 
      // case OBSTACLE_RIGHT: {
      //   angle = mpu9250_read_gyro_integration();
       
      //   if (is_button_pressed(&sensors)) {
      //       mpu9250_stop_gyro_integration();
      //       state = OFF;
      //   } else if (abs(angle.z_axis) >= 45) {
      //       kobukiDriveDirect(0, 0);
      //       // nrf_delay_ms(500);
      //       mpu9250_stop_gyro_integration();
      //       start_distance_encoder = sensors.leftWheelEncoder;
           
      //       // nrf_delay_ms(500);
      //       state = DRIVING;
      //   } else {
      //       kobukiDriveDirect(-50, 50);
      //       display_write("TURNING LEFT", DISPLAY_LINE_0);
      //       state = OBSTACLE_RIGHT;
      //       float turned = abs(angle.z_axis);
      //       snprintf(amount, 15, "%f d\n", turned);
      //       printf(amount);
      //       display_write(amount, DISPLAY_LINE_1);
      //   }
      //   break;
      // }
      // add other cases here
 
    }
  }
}
