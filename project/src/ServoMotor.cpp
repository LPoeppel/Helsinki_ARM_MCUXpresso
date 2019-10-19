/*
 * ServoMotor.cpp
 *
 *  Created on: 16.10.2019
 *      Author: Lukas
 */

#include "ServoMotor.h"
#include <cmath>

ServoMotor::ServoMotor(int port, int pin, int frq, int cycle): s_port(port), s_pin(pin)
{
	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O, port, pin);
	Chip_SCT_Init(LPC_SCT0);
	Chip_SCTPWM_Init(LPC_SCT0);

	LPC_SCT0->CONFIG |= (1 << 17);              		// two 16-bit timers, auto limit
	LPC_SCT0->CTRL_L |= (71) << 5;          			// set prescaler, SCTimer/PWM clock = 1 MHz
	LPC_SCT0->MATCHREL[0].L = PENFRQ - 1;    			// match 0 @ 1000/1MHz = (1 kHz PWM freq)
	LPC_SCT0->MATCHREL[1].L = PENCYCLE;        			// match 1 used for duty cycle (in 10 steps)
	LPC_SCT0->EVENT[0].STATE = 0xFFFFFFFF;      		// event 0 happens in all states
	LPC_SCT0->EVENT[0].CTRL = (1 << 12);        		// match 0 condition only
	LPC_SCT0->EVENT[1].STATE = 0xFFFFFFFF;     		 	// event 1 happens in all states
	LPC_SCT0->EVENT[1].CTRL = (1 << 0) | (1 << 12);    	// match 1 condition only
	LPC_SCT0->OUT[0].SET = (1 << 0);            		// event 0 will set SCTx_OUT0
	LPC_SCT0->OUT[0].CLR = (1 << 1);            		// event 1 will clear SCTx_OUT0
	LPC_SCT0->CTRL_L &= ~(1 << 2);              		// start
}

ServoMotor::ServoMotor(int port, int pin, int value): s_port(port), s_pin(pin)
{
	Chip_SWM_MovablePortPinAssign(SWM_SCT1_OUT0_O, port, pin);
	Chip_SCT_Init(LPC_SCT1);
	Chip_SCTPWM_Init(LPC_SCT1);

	LPC_SCT1->CONFIG |= (1 << 17);              		// two 16-bit timers, auto limit
	LPC_SCT1->CTRL_L |= (71) << 5;          			// set prescaler, SCTimer/PWM clock = 1 MHz
	LPC_SCT1->MATCH[0].L = value -1;    			// match 0 @ 1000/1MHz = (1 kHz PWM freq)
	LPC_SCT1->MATCHREL[0].L = value -1;        			// match 1 used for duty cycle (in 10 steps)
	LPC_SCT1->EVENT[0].STATE = 0xFFFFFFFF;      		// event 0 happens in all states
	LPC_SCT1->EVENT[0].CTRL = (1 << 12);        		// match 0 condition only
	LPC_SCT1->EVENT[1].STATE = 0xFFFFFFFF;     		 	// event 1 happens in all states
	LPC_SCT1->EVENT[1].CTRL = (1 << 0) | (1 << 12);    	// match 1 condition only
	LPC_SCT1->OUT[0].SET = (1 << 0);            		// event 0 will set SCTx_OUT0
	LPC_SCT1->OUT[0].CLR = (1 << 1);            		// event 1 will clear SCTx_OUT0
	LPC_SCT1->CTRL_L &= ~(1 << 2);              		// start
}


ServoMotor::~ServoMotor(){
}


void ServoMotor::move(int value){
    //calculating from bit to hz
    m_dutyCycle = ((m_lastValue /255) +1) *1000;

    //get the m_times and m_remain to increase or decrease dutycycle step by step
    m_times = (value > m_lastValue) ? ((value - m_lastValue) /5) : ((m_lastValue - value) /5);
    m_remain = (value > m_lastValue) ? (fmod((value - m_lastValue), 5)) : (fmod((m_lastValue - value), 5));

    if(value >= m_lastValue){
    	for(int i = 0; i < m_times; i++){
    	    m_dutyCycle += ((5/255) *1000);
    	    LPC_SCT0->MATCHREL[1].L = m_dutyCycle;
    	}
    	m_dutyCycle += ((m_remain/255) *1000);

    	//check duty cycle
		if (m_dutyCycle < 1003){
			m_dutyCycle = 1000;
		}else if (m_dutyCycle > 1996){
			m_dutyCycle = 2000;
		}
		LPC_SCT0->MATCHREL[1].L = m_dutyCycle;

    }else if(value < m_lastValue){
		for(int i = 0; i < m_times; i++){
			m_dutyCycle -= ((5/255) *1000.00);
			LPC_SCT0->MATCHREL[1].L = m_dutyCycle;
		}
		m_dutyCycle -= ((m_remain/255) *1000);
		if (m_dutyCycle < 1003){
			m_dutyCycle = 1000;
		}else if(m_dutyCycle > 1996){
			m_dutyCycle = 2000;
		}
		LPC_SCT0->MATCHREL[1].L = m_dutyCycle;
	}


}

void ServoMotor::laser(int value){
    //calculating from bit to hz
    m_dutyCycle = ((m_lastValue /255) +1) *1000;

    //get the m_times and m_remain to increase or decrease dutycycle step by step
    m_times = (value > m_lastValue) ? ((value - m_lastValue) /5) : ((m_lastValue - value) /5);
    m_remain = (value > m_lastValue) ? (fmod((value - m_lastValue), 5)) : (fmod((m_lastValue - value), 5));

    if(value >= m_lastValue){
    	for(int i = 0; i < m_times; i++){
    	    m_dutyCycle += ((5/255) *1000);
    	    LPC_SCT0->MATCHREL[1].L = m_dutyCycle;
    	}
    	m_dutyCycle += ((m_remain/255) *1000);

    	//check duty cycle
		if (m_dutyCycle < 1003){
			m_dutyCycle = 1000;
		}else if (m_dutyCycle > 1996){
			m_dutyCycle = 2000;
		}
		LPC_SCT0->MATCHREL[1].L = m_dutyCycle;

    }else if(value < m_lastValue){
		for(int i = 0; i < m_times; i++){
			m_dutyCycle -= ((5/255) *1000.00);
			LPC_SCT0->MATCHREL[1].L = m_dutyCycle;
		}
		m_dutyCycle -= ((m_remain/255) *1000);
		if (m_dutyCycle < 1003){
			m_dutyCycle = 1000;
		}else if(m_dutyCycle > 1996){
			m_dutyCycle = 2000;
		}
		LPC_SCT0->MATCHREL[1].L = m_dutyCycle;
	}


}
