/*
 * Motor.h
 *
 *  Created on: 20.09.2019
 *      Author: lukasp
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "DigitalIoPin.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdlib.h>
#include "config.h"
#endif /* MOTOR_H_ */

enum MotorType{X, Y, pen, laser};
enum Direction{left, right};

class Motor {
public:
	Motor(DigitalIoPin* power, DigitalIoPin* dir, DigitalIoPin* minSW, DigitalIoPin* maxSW, MotorType);
	Motor(DigitalIoPin* power, MotorType);
	virtual ~Motor();

	void calibrate();
	bool isCalibrated();
	void stepsToMM();

	void move();
    void move(bool);
    void move(int);
	void stop();
	void reverse();
	void setDir(bool);
    Direction getDirection();
    bool isLeft();

    bool isHit();
    void switchPower();

	int getSpeed();
	void setSpeed(int);

    void setPos(float);
    float getPos();
    void resetPos();

    void setPps(int);
    int getPps();

    void penUp(int);
    void penDown(int);

    //***************************************************//

    void RIT_start(int, int);
    void RIT_IRQHandler(void);

private:
    volatile uint32_t RIT_count;
    SemaphoreHandle_t sbRIT = NULL;

	MotorType s_type;

	DigitalIoPin* s_PWR;
	DigitalIoPin* s_DIR;
	DigitalIoPin* s_minLSW;
	DigitalIoPin* s_maxLSW;

	bool m_isCalibrated;
    float m_currentPosition;
	int m_pps;
    int m_steps;
	SemaphoreHandle_t m_stepSemaphore;

};
