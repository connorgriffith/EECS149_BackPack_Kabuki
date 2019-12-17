/* 
The pixy_ftl library

This code works with a Pixy2 camera and the NRF5832 to assist a kobuki robot in tracking
and following another kobuki robot.

Used in the "Follow The Leader" project. UC Berkeley EE149 Fall 2019.
*/

#include "pixy_ccc.h"
#include "assert.h"

uint16_t xpos;
float FTL_TOLERANCE = 0.25f;  // tolerance to dictate how "centered" the 
                              //leader must be to be considered straight ahead

float FTL_MIN;  // FTL_MIN threshold before being considered off center
float FTL_MAX;  // FTL_MAX threshold before being considered off center

typedef enum {
  LEADER_STRAIGHT,   // Indicates the leader robot is straight ahead
  LEADER_LEFT,       // Indicates the leader robot is to the left
  LEADER_RIGHT,      // Indicates the leader robot is to the right
  LEADER_NOT_VISIBLE // Indicates the leader robot is not in the camera frame
} LeaderDirection;

/*
Call this function first to perform all initializations necessary
*/
void pixy_ftl_init() {
  pixyInit(0);
  FTL_MIN = (0.5f-FTL_TOLERANCE)*(float)frameWidth;
  FTL_MAX = (0.5f+FTL_TOLERANCE)*(float)frameWidth;
}

/*
A function to change tolerance.
Inputs: t - a floate between 0 and 1 - Indicates how much to the left and right the leader is
allowed to be before being considered not straigh ahead. For example, if t = 0.2, the leader will be considered
straight ahead if it is in the middle 20% of the screen
*/
void pixy_ftl_change_tolerance(float t) {
  assert((t <= 1) && (t >= 0));
  FTL_TOLERANCE = t / 2;
  FTL_MIN = (0.5f-FTL_TOLERANCE)*(float)frameWidth;
  FTL_MAX = (0.5f+FTL_TOLERANCE)*(float)frameWidth;
}

/*
A function to locate the leader robot.
Returns a LeaderDirection which indicates which direction the follower robot should travel in
to continue following the leader robot.
This function also toggles LEDs on the Pixy2 for visual feedback:
  -Green means the leader is straight ahead
  -Red means the leader is off to one side
  -Blue means the leader is not visible within the camera frame
*/
LeaderDirection pixy_ftl_locate_leader() {
  getBlocks(true, CCC_SIG1, 10);
  xpos = blocks[0].m_x;
  //printf("%d\n", xpos);
  if (numBlocks == 0 || numBlocks > 3) {
    setLED(0, 0, 255);
    return LEADER_NOT_VISIBLE;
  } else {
    if (xpos < FTL_MIN) {
      setLED(255, 0, 0);
      return LEADER_LEFT;

    } else if (xpos > FTL_MAX) {
      setLED(255, 0, 0);
      return LEADER_RIGHT;
    } else {
      setLED(0, 255, 0);
      return LEADER_STRAIGHT;
    }
  }
}