/*
 * config.h
 *
 *  Created on: 15.10.2019
 *      Author: Lukas
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define motorXPwrPort      0
#define motorXPwrPin       27
#define motorXDirPort   0
#define motorXDirPin    28

#define motorYPwrPort      0
#define motorYPwrPin       24
#define motorYDirPort   1
#define motorYDirPin    0

#define motorMaxSpeed	700     // max speed, [step / s]
#define motorMinSpeed	300		// min speed, [step / s]
#define motorAccel		5		// acceleration rate, [step / s^2]

/* Limit switches */
#define limitXMinPort   0
#define limitXMinPin    0
#define limitXMaxPort   1
#define limitXMaxPin    3

#define limitYMinPort   0
#define limitYMinPin    29
#define limitYMaxPort   0
#define limitYMaxPin    9

/* Pen */
#define penPort         0
#define penPin          10


struct XYSetup{ //siehe M10
	int pen_up = 0;
	int pen_down = 0;
	int plottingSpeed = 0;
	float last_x_pos = 0;
	float last_y_pos = 0;
	int length_x = 0; //in mm
	int length_y = 0; //in mm
};



#endif /* CONFIG_H_ */
