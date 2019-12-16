/* 
An example of how to use the functions contained in pixy_ftl library

Place folder with this file in ~/buckler/software/apps/ or change make file to ensure this
can be flashed to a berkeley buckler when using the EE149 virtual machine.

Used in the "Follow The Leader" project. UC Berkeley EE149 Fall 2019.
*/

#include "pixy_ftl.h"

LeaderDirection direction;

int main(void) {
  pixy_ftl_init();
  pixy_ftl_change_tolerance(0.1f);
  while (1) {
  	direction = pixy_ftl_locate_leader();
  	switch (direction) {
  		case LEADER_RIGHT:
  		  printf("Turn Right!\n");
  		  break;
  		case LEADER_STRAIGHT:
  		  printf("Drive Straight!\n");
  		  break;
  		case LEADER_LEFT:
  		  printf("Turn left!\n");
  		  break;
  		case LEADER_NOT_VISIBLE:
  		  printf("Leader not visible...\n");
  }
  pixy_spi_delayms(100);
}
}

