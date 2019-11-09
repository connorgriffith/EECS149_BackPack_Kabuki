/*
 * states.h
 *
 *  Created on: Sep 22, 2018
 *      Author: shromonaghosh
 */

#ifndef STATES_H_
#define STATES_H_

#include <stdio.h>

typedef enum {
    OFF=0,
    FORWARD,
    BACKWARD,
    TURNING,
    RIGHT,
    LEFT
} states;

#endif /* STATES_H_ */
