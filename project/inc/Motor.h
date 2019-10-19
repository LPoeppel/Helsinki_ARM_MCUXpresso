/*
 * Motor.h
 *
 *  Created on: 20.09.2019
 *      Author: lukasp
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "config.h"
#include "DigitalIoPin.h"
#include "FreeRTOS.h"
#include "semphr.h"

#endif /* MOTOR_H_ */

class Motor{
public:
	Motor(DigitalIoPin* power, DigitalIoPin* dir, DigitalIoPin* minSW, DigitalIoPin* maxSW, int length);
	virtual ~Motor();

	void calibrate();
	bool isCalibrated();
	int mmToSteps(int);
	int stepsToMm(float);

    void move(int);
	void stop();
	void reverse();
	void setDir(bool isLeft);
	bool getDir();

    bool isHit();

	int getSpeed();
	void setSpeed(int);	//in %, for double speed int=200

    void setPos(int);
    int getPos();

    bool readMinLSW();
    bool readMaxLSW();

private:

	DigitalIoPin* s_PWR = NULL;
	DigitalIoPin* s_DIR = NULL;
	DigitalIoPin* s_minLSW = NULL;
	DigitalIoPin* s_maxLSW = NULL;

	const int s_length = 0;
	int s_steps = 0;

	bool m_isCalibrated = false;
    int m_currentPosition = 0;
	int m_speed = 0;

    volatile uint32_t RIT_count = 0;
    SemaphoreHandle_t sbRIT = xSemaphoreCreateBinary();
    SemaphoreHandle_t sbCalibrate = xSemaphoreCreateBinary();
	void moveSteps(int);
    void RIT_start(int, int);
    void RIT_IRQHandler(void);

    //just for calibration
    void RIT_calibrate();
    void RIT_IRQcalibrate(void);

};
