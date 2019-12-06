// // BLE RX app
// //
// // Receives BLE advertisements with data

// #include <stdbool.h>
// #include <stdint.h>
// #include "nrf.h"
// #include "app_util.h"
// #include "simple_ble.h"



// #include "app_error.h"
// #include "app_timer.h"
// #include "nrf_delay.h"
// #include "nrf_gpio.h"
// #include "nrf_log.h"
// #include "nrf_log_ctrl.h"
// #include "nrf_log_default_backends.h"
// #include "nrf_pwr_mgmt.h"
// #include "nrf_drv_spi.h"

// #include "buckler.h"
// #include "display.h"
// #include "kobukiActuator.h"
// #include "kobukiSensorPoll.h"
// #include "kobukiSensorTypes.h"
// #include "kobukiUtilities.h"
// #include "mpu9250.h"


// // BLE configuration
// // This is mostly irrelevant since we are scanning only
// static simple_ble_config_t ble_config = {
//         // BLE address is c0:98:e5:49:00:00
//         .platform_id       = 0x49,    // used as 4th octet in device BLE address
//         .device_id         = 0x0005,  // Last two octets of device address
//         .adv_name          = "EE149", // irrelevant in this example
//         .adv_interval      = MSEC_TO_UNITS(100, UNIT_0_625_MS), // send a packet once per second (minimum is 20 ms)
//         .min_conn_interval = MSEC_TO_UNITS(50, UNIT_1_25_MS), // irrelevant if advertising only
//         .max_conn_interval = MSEC_TO_UNITS(80, UNIT_1_25_MS), // irrelevant if advertising only
// };
// simple_ble_app_t* simple_ble_app;
// NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// const uint8_t LEADER_address[6] = {0x00, 0x00, 0x49, 0xE5, 0x98, 0xC0};
// // TODO: implement BLE advertisement callback
// void ble_evt_adv_report(ble_evt_t const* p_ble_evt) {

//   uint8_t* address = p_ble_evt->evt.gap_evt.params.adv_report.peer_addr.addr;
//   if (address[0] == 0x00 && address[1] == 0x00 && address[5] == 0xC0 && address[3] == 0xE5) {
//     //printf("%x:%x:%x:%x:%x:%x\n", address.addr[0], address.addr[1], address.addr[2], address.addr[3], address.addr[4], address.addr[5]);
//       //ble_data_t data = p_ble_evt->evt.gap_evt.params.adv_report.data;
//       // uint8_t* data_addr = data.p_data;
//       // uint16_t data_len = data.len;

//       //display_write("GOT ADV", DISPLAY_LINE_1);

//       for (int i = 0; i < 6; i++)
//       {
//         if (LEADER_address[i] != address[i]) {
//           return;
//         }
//       }

// //We are trying to decode and access signed data...shouldnt this be signed data pointer
//      uint8_t* data = p_ble_evt->evt.gap_evt.params.adv_report.data.p_data;
//      //printf("Data: %d\n", *data);
//      uint32_t length;
//      int16_t* dataPtr;
//      bool flag = false;

//      for (int i = 0; i < 31;)
//      {
//        length = data[i];
//        if (data[i + 1] == 0xFF) {
//           dataPtr = &data[i + 4];
//           flag = true;
//           break;
//        } 
//        i = i + (length + 1);
//      }

//      if (!flag)
//      {
//        return;
//      } else {
//       // printf("Length: %d\n", length);
//       // printf("First int: %d\n", *dataPtr);
//       char buf[16];
//       snprintf(buf, 16, "%d", *dataPtr);
//       //printf("%s\n", dataPtr);
//       display_write(buf, DISPLAY_LINE_0);
//       //printf("%.*s\n", (length -3), dataPtr);
//      }
//     }
// }

// int main(void) {

//   // initialize display
//   nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
//   nrf_drv_spi_config_t spi_config = {
//     .sck_pin = BUCKLER_LCD_SCLK,
//     .mosi_pin = BUCKLER_LCD_MOSI,
//     .miso_pin = BUCKLER_LCD_MISO,
//     .ss_pin = BUCKLER_LCD_CS,
//     .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
//     .orc = 0,
//     .frequency = NRF_DRV_SPI_FREQ_4M,
//     .mode = NRF_DRV_SPI_MODE_2,
//     .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
//   };
//   ret_code_t error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
//   APP_ERROR_CHECK(error_code);
//   display_init(&spi_instance);
//   printf("Display initialized!\n");

//     // initialize i2c master (two wire interface)
//   nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
//   i2c_config.scl = BUCKLER_SENSORS_SCL;
//   i2c_config.sda = BUCKLER_SENSORS_SDA;
//   i2c_config.frequency = NRF_TWIM_FREQ_100K;
//   error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
//   APP_ERROR_CHECK(error_code);
//   mpu9250_init(&twi_mngr_instance);
//   printf("IMU initialized!\n");

//   // initialize Kobuki
//   kobukiInit();
//   printf("Kobuki initialized!\n");

//   // Setup BLE
//   // Note: simple BLE is our own library. You can find it in `nrf5x-base/lib/simple_ble/`
//   simple_ble_app = simple_ble_init(&ble_config);
//   advertising_stop();

//   // TODO: Start scanning
//   //BLE Event above will now be called whenever there is an event to decode
//   scanning_start();

//   while(1) {
//     // Sleep while SoftDevice handles BLE
//     power_manage();
//     display_write("INIT", DISPLAY_LINE_1);
//   }
// }





// BLE RX app
//
// Receives BLE advertisements with data

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_util.h"
#include "simple_ble.h"

#include "states.h"

#include "app_error.h"
#include "app_timer.h"
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


// void __attribute__((weak)) scanning_stop(void) {
//     // static ble_gap_scan_params_t m_scan_params = {
//     //     .active            = false, // passive scanning (no scan response)
//     //     .interval          = MSEC_TO_UNITS(100, UNIT_0_625_MS), // interval 100 ms
//     //     .window            = MSEC_TO_UNITS(100, UNIT_0_625_MS), // window 100 ms
//     //     .timeout           = BLE_GAP_SCAN_TIMEOUT_UNLIMITED,
//     //     .scan_phys         = BLE_GAP_PHY_1MBPS,
//     //     .filter_policy     = BLE_GAP_SCAN_FP_ACCEPT_ALL,
//     // };

//     ret_code_t err_code = sd_ble_gap_scan_stop();
//     APP_ERROR_CHECK(err_code);
//     printf("INSIDE SCANNING STOP");
// }

// BLE configuration
// This is mostly irrelevant since we are scanning only
static simple_ble_config_t ble_config = {
        // BLE address is c0:98:e5:49:00:00
        .platform_id       = 0x49,    // used as 4th octet in device BLE address
        .device_id         = 0x0005,  // Last two octets of device address
        .adv_name          = "EE149", // irrelevant in this example
        .adv_interval      = MSEC_TO_UNITS(100, UNIT_0_625_MS), // send a packet once per second (minimum is 20 ms)
        .min_conn_interval = MSEC_TO_UNITS(50, UNIT_1_25_MS), // irrelevant if advertising only
        .max_conn_interval = MSEC_TO_UNITS(80, UNIT_1_25_MS), // irrelevant if advertising only
};
simple_ble_app_t* simple_ble_app;

NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

const uint8_t LEADER_address[6] = {0x00, 0x00, 0x49, 0xE5, 0x98, 0xC0};

states curr_state;
states next_state;
KobukiSensors_t sensors = {0};
int turn_right = -1;  // -1 for turn right; 1 for turn left
int adv_angle = 0;
int angle_turned = 0;
int initial_angle = 0;

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

// TODO: implement BLE advertisement callback
void ble_evt_adv_report(ble_evt_t const* p_ble_evt) {
  uint8_t* address = p_ble_evt->evt.gap_evt.params.adv_report.peer_addr.addr;
  if (address[0] == 0x00 && address[1] == 0x00 && address[5] == 0xC0 && address[3] == 0xE5) {


    for (int i = 0; i < 6; i++) {
      if (LEADER_address[i] != address[i])
        return;
    }

    uint8_t* data = p_ble_evt->evt.gap_evt.params.adv_report.data.p_data;
    uint32_t length;
    int16_t* dataPtr;
    bool flag = false;

    for (int i = 0; i < 31;) {
      flag = data[i+1] == 0xFF;
      length = data[i];
      if (flag) {
        // dataPtr = &data[i+4];
        dataPtr = &data[i+3];  ///THIS WAS A +4 FOR IT TO WORK PROPERLY.......!!!!!!!!!!!!!!!!!!!!!!

        break;
      }
      i += (length + 1);
    }

    if (!flag) {
      return;
    } else {
      //printf("Length: %d\n", length);
      //printf("First int: %d\n", *dataPtr);
      adv_angle = *dataPtr;
      turn_right = (adv_angle != 0) ? (adv_angle / abs(adv_angle)) : 1;
      if (turn_right == -1) {
        // curr_state = BRAKE;
        // next_state = TURNING;
        kobukiDriveDirect(0, 0);
        curr_state = TURNING;
        mpu9250_start_gyro_integration();
      }
      // char buf[16];
      // snprintf(buf, 16, "%d", *dataPtr);
      // display_write(buf, DISPLAY_LINE_1);

      //stopScan();
      

      printf("Should only fire when get advertisement\n");
    }
  }
}


int main(void) {

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
  ret_code_t error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
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

  // Setup BLE
  // Note: simple BLE is our own library. You can find it in `nrf5x-base/lib/simple_ble/`
  simple_ble_app = simple_ble_init(&ble_config);
  advertising_stop();

  curr_state = OFF;
  next_state = OFF;

  int delay_count = 0;
  

  // TODO: Start scanning
  //BLE Event above will now be called whenever there is an event to decode
  scanning_start();

  while(1) {
    // Sleep while SoftDevice handles BLE
    // power_manage();
    // display_write("INIT", DISPLAY_LINE_1);
    //printf("About to enter kobukiSensorPoll\n");
   // kobukiSensorPoll(&sensors);
    print_state(curr_state);



    switch(curr_state) {
      case OFF: {
        // if (is_button_pressed(&sensors)) {
        //   angle_turned = 0;
        //   delay_count = 0;
        //   curr_state = DRIVING;
        //   //scanning_start();
        // } else {
        //   curr_state = OFF;
        //   kobukiDriveDirect(0,0);
        //   //scanning_start();
        // }
         curr_state = OFF;
	      kobukiDriveDirect(0,0);
	      //printf("IN OFF STATE");
        break;
      }

      case DRIVING: {
        // if (is_button_pressed(&sensors)) {
        //   curr_state = OFF;
        // } else {
        //   curr_state = DRIVING;
        //   kobukiDriveDirect(100,100);
          //scanning_start();


        //DO NOTHING RN
        display_write("DO NOTHING", DISPLAY_LINE_1);
        
        break;
      }

      case TURNING: {
        //printf("TURNING");
        //scanning_stop();

        printf("SCANNING STOP RETURNED\n");
        angle_turned = (int) mpu9250_read_gyro_integration().z_axis;

        // if (is_button_pressed(&sensors)) {
        //   curr_state = OFF;
        //   mpu9250_stop_gyro_integration();
        // } 
        if (angle_turned  == adv_angle) {
          // curr_state = BRAKE;
          // next_state = DRIVING;
          angle_turned = 0;
          kobukiDriveDirect(0, 0);
          curr_state = OFF;
          //printf("GOING TO OFF STATE\n");
          mpu9250_stop_gyro_integration();
          //initial_angle = angle_turned;
          //printf("LEAVING\n");
          //scanning_start();


        } else {
          curr_state = TURNING;
          kobukiDriveDirect(50, -50);
          //angle_turned = (int) mpu9250_read_gyro_integration().z_axis;

          char buf[16];
          snprintf(buf, 16, "%d", angle_turned);
          display_write(buf, DISPLAY_LINE_1);
        }
        break;
      }

      // case BRAKE: {
      //   if (is_button_pressed(&sensors)) {
      //     curr_state = OFF;
      //     next_state = OFF;
      //   } else if (delay_count < 5) {
      //     kobukiDriveDirect(0,0);
      //     delay_count++;
      //     curr_state = BRAKE;
      //   } else {
      //     delay_count = 0;
      //     curr_state = next_state;
      //     if (curr_state == TURNING) {
      //       mpu9250_start_gyro_integration();
      //     }
      //   }
      //   break;
      // }

    }
  }
}