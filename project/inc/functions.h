/*
 * functions.h
 *
 *  Created on: 15.10.2019
 *      Author: Lukas
 */

#include "config.h"
#include "Motor.h"
#include "ServoMotor.h"

#include <string.h>
#include <math.h>

#ifndef USB_CDC_FUNCTIONS_H_
#define USB_CDC_FUNCTIONS_H_

enum cmd{
	G1,
	G28,
	M1,
	M2,
	M4,
	M5,
	M10,
	M11,
	Z0
};
struct command_struct{
	cmd type;
	char input[64];
};
struct config{
	char type[3];
	int height;
	int width;
	int xDir;
	int yDir;
	int plottingSpeed;
	int penUp;
	int penDown;
};

void prvSetupHardware(void);
command_struct parse(QueueHandle_t&);
void run(command_struct);

void mirror(int);
int isNotEmpty(char*);
void moveTo(int);
void moveTo(int, int, bool);
void sendM10();
void sendM11(int, int, int, int);

#endif /* USB_CDC_FUNCTIONS_H_ */
