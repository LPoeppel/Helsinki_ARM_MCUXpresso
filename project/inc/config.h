/*
 * config.h
 *
 *  Created on: 15.10.2019
 *      Author: Lukas
 */

#ifndef CONFIG_H_
#define CONFIG_H_

/* X & Y Motors */
#define XPWRPORT		0
#define XPWRPIN			27
#define XDirPORT		0
#define XDIRPIN			28
#define XDIRORIGIN		1		//!?!?!??!?!?!?!
#define LENGTHX			380
#define LSWXMINPORT   	0
#define LSWXMINPIN    	0
#define LSWXMAXPORT   	1
#define LSWXMAXPIN		3

#define YPWRPORT		0
#define YPWRPIN			24
#define YDIRPORT   		1
#define YDIRPIN    		0
#define YDIRORIGIN		0		//!?!?!??!?!?!?!
#define LENGTHY			310
#define LSWYMINPORT   	0
#define LSWYMINPIN    	29
#define LSWYMAXPORT   	0
#define LSWYMAXPIN    	9

#define OFFSET			10

#define SPEEDMAX		700     // max speed, [step / s]
#define SPEEDMIN		300		// min speed, [step / s]
#define ACCELERATION	5		// acceleration rate, [step / s^2]

/* Pen & Laser Motors */
#define PENPORT         0
#define PENPIN          10
#define PENFRQ 			20000
#define PENCYCLE 		1000

#define LASERPORT       0
#define LASERPIN        12

#endif /* CONFIG_H_ */
