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

#include "pixy_ftl.h"

// Create a timer
APP_TIMER_DEF(adv_timer);
APP_TIMER_DEF(pixy_timer);

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// global variables
KobukiSensors_t sensors = {0};

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:49:xx:xx
        .platform_id       = 0x49,    // used as 4th octect in device BLE address
        .device_id         = 0x0005, // TODO: replace with your lab bench number
        .adv_name          = "KOBUKI", // used in advertisements if there is room
        .adv_interval      = MSEC_TO_UNITS(100, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
};

simple_ble_app_t* simple_ble_app;


const uint8_t LEADER_address[6] = {0x00, 0x00, 0x49, 0xE5, 0x98, 0xC0};

states curr_state;
states next_state;
int turn_right = -1;  // -1 for turn right; 1 for turn left
int adv_angle = 0;
int angle_turned = 0;
int initial_angle = 0;
bool readyForNewAdv = true;
bool comingFromWaiting = false;
uint16_t start_distance_encoder = 0;
bool reinitStartEncorderVal = false;
int stopLeaderVal = 250;
int startLeaderAgain = 240;
int kobukiLeftWheelSpeedCmd = 0;
int kobukiRightWheelSpeedCmd = 0;
LeaderDirection direction;
bool pixyTimerREINIT = true;
bool pixyTimerGoodToFire = true;

void print_state(states current_state){
  switch(current_state){
    case OFF:
      //display_write("OFF", DISPLAY_LINE_0);
      break;
    case DRIVING:
      //display_write("DRIVING", DISPLAY_LINE_0);
      break;
    case TURNING:
      //display_write("TURNING", DISPLAY_LINE_0);
      break;
    default:
      break;
  }
}

static void timer_callback (void * p_context) {
  printf("Called advertising_stop!!!!!!!!!!!!\n");
  advertising_stop();
  pixyTimerREINIT = true;
}

static void pixy_timer_callback (void * p_context) {
  printf("Pixy timer fired\n");
  direction = pixy_ftl_locate_leader();

    switch (direction) {
      case LEADER_RIGHT:
        //printf("Turn Right!\n");
        kobukiLeftWheelSpeedCmd = 115;
        kobukiRightWheelSpeedCmd = 100;
        break;
      case LEADER_STRAIGHT:
        kobukiLeftWheelSpeedCmd = 100;
        kobukiRightWheelSpeedCmd = 100;
        //printf("Drive Straight!\n");
        break;
      case LEADER_LEFT:\
        kobukiLeftWheelSpeedCmd = 100;
        kobukiRightWheelSpeedCmd = 115;
        //printf("Turn left!\n");
        break;
      case LEADER_NOT_VISIBLE:
        // kobukiLeftWheelSpeedCmd = 0;
        // kobukiRightWheelSpeedCmd = 0;
        printf("Leader not visible...\n");

    }
}

void ble_evt_adv_report(ble_evt_t const* p_ble_evt) {
  uint8_t* address = p_ble_evt->evt.gap_evt.params.adv_report.peer_addr.addr;
  if (address[0] == 0x00 && address[1] == 0x00 && address[5] == 0xC0 && address[3] == 0xE5 && readyForNewAdv) {


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
        dataPtr = &data[i+4];  

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
       if (adv_angle == 250)
       {
         //This is when the kobuki needs to stop and wait for the leader to finish its turn
         curr_state = OFF;
         printf("Got the adv to stay still\n angle: %d\n", adv_angle);

        //Stop the pixy timer for turning
        app_timer_stop(pixy_timer);
        setLED(0, 0, 0);

       }

       else if (turn_right == -1) {
        // curr_state = BRAKE;
        // next_state = TURNING;
        //kobukiDriveDirect(0, 0);
        printf("Will drive 0.4 m then turn: %d\n", adv_angle);

        kobukiLeftWheelSpeedCmd = 100;
        kobukiRightWheelSpeedCmd = 100;
        kobukiDriveDirect(kobukiLeftWheelSpeedCmd, kobukiRightWheelSpeedCmd);
        comingFromWaiting = true;
        reinitStartEncorderVal = true;
        curr_state = DRIVING;
        //curr_state = TURNING;
        
        // mpu9250_start_gyro_integration();
      } else {
        printf("This should never fire\n");
      }
      // char buf[16];
      // snprintf(buf, 16, "%d", *dataPtr);
      // display_write(buf, DISPLAY_LINE_1);

      //stopScan();
      

      
    }
  }
}



const float CONVERSION = 0.00008529;
   
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

  simple_ble_adv_only_name();


  

//______________Take out if no work____________
  // Start Advertising
  

  // initialize LEDs
  // nrf_gpio_pin_dir_set(23, NRF_GPIO_PIN_DIR_OUTPUT);
  // nrf_gpio_pin_dir_set(24, NRF_GPIO_PIN_DIR_OUTPUT);
  // nrf_gpio_pin_dir_set(25, NRF_GPIO_PIN_DIR_OUTPUT);

  // initialize display
  // nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
  // nrf_drv_spi_config_t spi_config = {
  //   .sck_pin = BUCKLER_LCD_SCLK,
  //   .mosi_pin = BUCKLER_LCD_MOSI,
  //   .miso_pin = BUCKLER_LCD_MISO,
  //   .ss_pin = BUCKLER_LCD_CS,
  //   .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
  //   .orc = 0,
  //   .frequency = NRF_DRV_SPI_FREQ_4M,
  //   .mode = NRF_DRV_SPI_MODE_2,
  //   .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
  // };
  // error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  // APP_ERROR_CHECK(error_code);
  // display_init(&spi_instance);
  // printf("Display initialized!\n");

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
  // simple_ble_app = simple_ble_init(&ble_config);
  // advertising_stop();

  curr_state = OFF;
  next_state = OFF;

  printf("Initializing pixy\n");
  pixy_ftl_init();
  printf("Changing pixy tolerance\n");
  pixy_ftl_change_tolerance(0.2f);
  printf("Done Initializing pixy\n");

  int delay_count = 0;
  bool inRange = true;

  //BLE timer
  app_timer_init();
  error_code = app_timer_create(&adv_timer, APP_TIMER_MODE_SINGLE_SHOT, (app_timer_timeout_handler_t) timer_callback);
  APP_ERROR_CHECK(error_code);
  

  //Pixy timer
  error_code = app_timer_create(&pixy_timer, APP_TIMER_MODE_REPEATED, (app_timer_timeout_handler_t) pixy_timer_callback);
  APP_ERROR_CHECK(error_code);


  advertising_stop();

  // TODO: Start scanning
  //BLE Event above will now be called whenever there is an event to decode
  scanning_start();

   //Pixy timer stuff
  // app_timer_create(&pixy_timer, APP_TIMER_MODE_REPEATED, (app_timer_timeout_handler_t) pixy_timer_callback);
  // app_timer_start(pixy_timer, APP_TIMER_TICKS(500), NULL); 




/*
  _________________If the pixy camera does not work___________________

  Idea:  I can periodically send a couple gyroscope values and verify is the follower and leader are the same values.

  If (leaderGyro < followerGyro) {
    Turn follower to the right
  }

  else if (leaderGyro > followerGyro) {
   Turn Follower to the left
  }

*/




  while(1) {

    kobukiSensorPoll(&sensors);
    print_state(curr_state);



     switch(curr_state) {
      case OFF: {


        if (is_button_pressed(&sensors)) {
          curr_state = DRIVING;

          start_distance_encoder = sensors.leftWheelEncoder;

          kobukiLeftWheelSpeedCmd = 100;
          kobukiRightWheelSpeedCmd = 100;
          kobukiDriveDirect(kobukiLeftWheelSpeedCmd, kobukiRightWheelSpeedCmd);

        } else {
          curr_state = OFF;
          kobukiLeftWheelSpeedCmd = 0;
          kobukiRightWheelSpeedCmd = 0;
          kobukiDriveDirect(kobukiLeftWheelSpeedCmd, kobukiRightWheelSpeedCmd);
        }

        break; // each case needs to end with break!


      }

      case DRIVING: {

        //Reinit pixy timer if coming from Turning
        if (pixyTimerREINIT)
        {
          //app_timer_create(&pixy_timer, APP_TIMER_MODE_REPEATED, (app_timer_timeout_handler_t) pixy_timer_callback);
        app_timer_start(pixy_timer, APP_TIMER_TICKS(500), NULL); 
        pixyTimerREINIT = false;
        }

        //Adjust the speed if leader not in range
        // if (!inRange)
        // {
        //  kobukiLeftWheelSpeedCmd = 130;
        //  kobukiRightWheelSpeedCmd = 130;
        // } else {
        //  kobukiLeftWheelSpeedCmd = 100;
        //  kobukiRightWheelSpeedCmd = 100;
        // }


        //Follower move up to leader's turning location 
        if (reinitStartEncorderVal && comingFromWaiting)
        {
          start_distance_encoder = sensors.leftWheelEncoder;
          reinitStartEncorderVal = false;
        }

        kobukiDriveDirect(kobukiLeftWheelSpeedCmd, kobukiRightWheelSpeedCmd);


        float traveled = measure_distance(sensors.leftWheelEncoder, start_distance_encoder);

        //This check is to eliminate the -5.5 returned at times by measure_distance.  
        //Somehow the wheelEncoder is reading 5.5 at times. But then after the 
        //...measure_distance function is called the wheelEncoder gets zeroed
        if (abs(traveled) > 4.5)
        {
          start_distance_encoder = sensors.leftWheelEncoder;
          traveled = measure_distance(sensors.leftWheelEncoder, start_distance_encoder);
        
        }

        // transition logic
        if (is_button_pressed(&sensors)) {
          mpu9250_stop_gyro_integration();
          curr_state = OFF;
        } 
      
       else if (traveled >= 0.6 && comingFromWaiting) {
        printf("Should only happen after traveled 0.3m\n\n");
          curr_state = TURNING;
          mpu9250_start_gyro_integration();
          comingFromWaiting = false;

          simple_ble_adv_manuf_data((uint8_t*) &stopLeaderVal, 4);
          //app_timer_create(&adv_timer, APP_TIMER_MODE_SINGLE_SHOT, (app_timer_timeout_handler_t) timer_callback);
          app_timer_start(adv_timer, APP_TIMER_TICKS(1000), NULL); 
          //pixyTimerGoodToFire = false;

       }


       else {
          curr_state = DRIVING;
     
          //char buf1[16];
          // snprintf(buf1, 16, "%f", traveled);
          // display_write(buf1, DISPLAY_LINE_1);

         
          //kobukiDriveDirect(kobukiLeftWheelSpeedCmd, kobukiRightWheelSpeedCmd);
  
          

        }
        break; // each case needs to end with break!
      }


      case TURNING: {
       
        //Stop pixy timer just while turning

        angle_turned = (int) mpu9250_read_gyro_integration().z_axis;

      
        if (angle_turned  <= adv_angle) {
          angle_turned = 0;
          kobukiLeftWheelSpeedCmd = 100;
          kobukiRightWheelSpeedCmd = 100;
          //kobukiDriveDirect(kobukiLeftWheelSpeedCmd, kobukiRightWheelSpeedCmd);
          curr_state = DRIVING;
          mpu9250_stop_gyro_integration();
          start_distance_encoder = sensors.leftWheelEncoder;

          simple_ble_adv_manuf_data((uint8_t*) &startLeaderAgain, 4);
          //app_timer_create(&adv_timer, APP_TIMER_MODE_SINGLE_SHOT, (app_timer_timeout_handler_t) timer_callback);
          app_timer_start(adv_timer, APP_TIMER_TICKS(500), NULL); 
          
          //logic for when to allow pixy to execute again
          pixyTimerGoodToFire = false;
          pixyTimerREINIT = false;
         

        } else {
          curr_state = TURNING;
          kobukiDriveDirect(50, -50);

          //char buf[16];
          // snprintf(buf, 16, "%d", angle_turned);
          // display_write(buf, DISPLAY_LINE_1);
        }

        break;
      }
    }
  }
}
