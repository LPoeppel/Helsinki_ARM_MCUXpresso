/*
 * ServoMotor.h
 *
 *  Created on: 16.10.2019
 *      Author: Lukas
 */

#ifndef SERVOMOTOR_H_
#define SERVOMOTOR_H_


#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include "config.h"
#include "sct_15xx.h"
#include "swm_15xx.h"

class ServoMotor{
	public:
		ServoMotor(int Port, int Pin, int frq, int cycle);
		ServoMotor(int Port, int Pin, int maxValue);
		virtual ~ServoMotor();
		virtual void move(int);
		virtual void laser(int);

	private:
		const int s_port;
		const int s_pin;

		int m_times;
		double m_remain;
		double m_lastValue;
		double m_dutyCycle;
};

#endif /* SERVOMOTOR_H_ */
