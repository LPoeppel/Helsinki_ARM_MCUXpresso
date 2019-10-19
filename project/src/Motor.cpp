/*
 * Motor.cpp
 *
 *  Created on: 20.09.2019
 *      Author: lukasp
 *
 */
#include "Motor.h"

#pragma GCC diagnostic push
Motor::Motor(DigitalIoPin* power, DigitalIoPin* dir,
		DigitalIoPin* minSW, DigitalIoPin* maxSW, int length):
            s_PWR(power), s_DIR(dir), s_minLSW(minSW),
			s_maxLSW(maxSW), s_length(10*length)
{
    s_PWR->write(false);
	s_DIR->write(false);
    m_isCalibrated = false;

    s_steps = 0;
	m_currentPosition = 0.0;
	m_speed = ((SPEEDMAX-SPEEDMIN)/2)+SPEEDMIN;
}

Motor::~Motor(){
	delete s_PWR;
	delete s_DIR;
	delete s_minLSW;
	delete s_maxLSW;
}

//Calibration
void Motor::calibrate() {
    stop();
    int temp = 0;
    this->setDir(false);

	while(!s_maxLSW->read()){
		this->setDir(true);
		moveSteps(1);
	}
	s_steps = 0;
	while (!s_minLSW->read()) {
		this->setDir(false);
		moveSteps(1);
		s_steps++;
	}
	moveSteps(OFFSET);

	this->setPos(0);
	s_steps = temp-(OFFSET*2);
	m_isCalibrated = true;
}
bool Motor::isCalibrated(){
	return m_isCalibrated;
}

int Motor::mmToSteps(int value){
	float factor = s_length/s_steps;
	return (value*factor);
}

int Motor::stepsToMm(float value){
	float factor = s_steps/s_length;
	return (value * factor);
}

//Movement
void Motor::move(int mm){
	int steps = mmToSteps(mm);
	if((((this->getPos()+mm > (s_length*0.8) && this->getDir() == (!XDIRORIGIN || !YDIRORIGIN))) || ((this->getPos()-mm < (s_length*0.2))&& this->getDir() == (XDIRORIGIN || YDIRORIGIN))) && (m_speed > ((SPEEDMAX-SPEEDMIN)/2)+SPEEDMIN)){
			m_speed = (((SPEEDMAX-SPEEDMIN)/2)+SPEEDMIN);
		}
	if(this->getPos()+mm >= (s_length*0.9) || this->getPos()-mm <= (s_length*0.1)){
			m_speed = SPEEDMIN;
		}
	RIT_start(steps, m_speed);
	if(this->getDir() == (!XDIRORIGIN || !YDIRORIGIN)){
		this->setPos((this->getPos())+mm);
	}else if(this->getDir() == (XDIRORIGIN || YDIRORIGIN)){
		this->setPos((this->getPos())-mm);
	}
}

void Motor::stop(){
	s_PWR->write(false);
}

void Motor::reverse(){
	s_DIR->write(!s_DIR->read());
}

void Motor::setDir(bool value){
    s_DIR->write(value);
}
bool Motor::getDir(){
	return (s_DIR->read());
}

bool Motor::isHit(){
    return ( (s_minLSW->read()) || (s_maxLSW->read()) );
}

void Motor::setPos(int pos){
    m_currentPosition = pos;
}
int Motor::getPos(){
	return m_currentPosition;
}

int Motor::getSpeed(){
	int speed = ((m_speed-SPEEDMIN)*100)/(SPEEDMAX-SPEEDMIN);
	return speed;
}
void Motor::setSpeed(int value){
	m_speed = ((value* (SPEEDMAX-SPEEDMIN) /100))  +SPEEDMIN;
}

bool Motor::readMinLSW(){
	return (s_minLSW->read());
}
bool Motor::readMaxLSW(){
	return (s_maxLSW->read());
}
//*******************//
//******private******//
//*******************//

void Motor::moveSteps(int value){
	RIT_start(1, m_speed);
}

//count amount of turns, us in time
void Motor::RIT_start(int count, int us){
	 uint64_t cmp_value;
	 // Determine approximate compare value based on clock rate and passed interval
	 cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
	 // disable timer during configuration
	 Chip_RIT_Disable(LPC_RITIMER);
	 RIT_count = count;
	 // enable automatic clear on when compare value==timer value
	 // this makes interrupts trigger periodically
	 Chip_RIT_EnableCompClear(LPC_RITIMER);
	 // reset the counter
	 Chip_RIT_SetCounter(LPC_RITIMER, 0);
	 Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	 // start counting
	 Chip_RIT_Enable(LPC_RITIMER);
	 // Enable the interrupt signal in NVIC (the interrupt controller)
	 NVIC_EnableIRQ(RITIMER_IRQn);
	 // wait for ISR to tell that we're done
	 if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		 // Disable the interrupt signal in NVIC (the interrupt controller)
		 NVIC_DisableIRQ(RITIMER_IRQn);
	 }else {
		 // unexpected error
	 }
}

extern "C" {
void Motor::RIT_IRQHandler(void){

	 // This used to check if a context switch is required
	 portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	 // Tell timer that we have processed the interrupt.
	 // Timer then removes the IRQ until next match occurs
	 Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	 if(RIT_count > 0) {
		 RIT_count--;
		 if(isHit()){
			 s_PWR->write(false);
			 RIT_count = 0;
		 //}else if(){
		 }else if(RIT_count%2){ s_PWR->write(true); }
			 else{ s_PWR->write(false); }

	 }else{
		 Chip_RIT_Disable(LPC_RITIMER); // disable timer
		 // Give semaphore and set context switch flag if a higher priority task was woken up
		 xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
	 }
	 // End the ISR and (possibly) do a context switch
	 portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}

/*
//count amount of turns, us in time
void Motor::RIT_calibrate(){

	 uint64_t cmp_value = ((uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) m_speed / 1000000);
	 Chip_RIT_Disable(LPC_RITIMER);
	 RIT_count = 5;
	 Chip_RIT_EnableCompClear(LPC_RITIMER);
	 Chip_RIT_SetCounter(LPC_RITIMER, 0);
	 Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	 Chip_RIT_Enable(LPC_RITIMER);
	 NVIC_EnableIRQ(RITIMER_IRQn);
	 if(xSemaphoreTake(sbCalibrate, portMAX_DELAY) == pdTRUE) {
		 NVIC_DisableIRQ(RITIMER_IRQn);
	 }else {
		 // unexpected error
	 }
}

extern "C" {
void Motor::RIT_IRQcalibrate(void){
	 portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	 Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	 if(RIT_count > 0) {
		 RIT_count--;
		 if(isHit()){
			 s_PWR->write(false);
			 RIT_count = 0;
		 }else{
			 if(RIT_count%2){ s_PWR->write(true); }
			 else{ s_PWR->write(false); }
		 }
		 s_steps+=1;
		 RIT_count+=2;

	 }else{
		 Chip_RIT_Disable(LPC_RITIMER); // disable timer
		 xSemaphoreGiveFromISR(sbCalibrate, &xHigherPriorityWoken);
	 }
	 portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}
*/
