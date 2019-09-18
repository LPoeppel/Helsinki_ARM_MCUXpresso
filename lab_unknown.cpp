/*
===============================================================================
 Name        : main.c
 Author      : $lpoeppel
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include "DigitalIoPin.h"
#include <cr_section_macros.h>

// TODO: insert other include files here
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "ITM_write.h"

#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>
// TODO: insert other definitions and declarations here

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false); //red
	Board_LED_Set(1, false); //green
	Board_LED_Set(2, false); //blue

	// initialize RIT (= enable clocking etc.)
	Chip_RIT_Init(LPC_RITIMER);
	// set the priority level of the interrupt
	// The level must be equal or lower than the maximum priority specified in FreeRTOS config
	// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );

}
#define speed 10

SemaphoreHandle_t binary = NULL;
DigitalIoPin *rightSW;
DigitalIoPin *leftSW;
DigitalIoPin *dir;
DigitalIoPin *power;
DigitalIoPin *SW1;
DigitalIoPin *SW2;
DigitalIoPin *SW3;

volatile uint32_t RIT_count;
SemaphoreHandle_t sbRIT = NULL;
int clockrate = 400; //stepper motor speed
int steps = 0; //steps

void stop(){
	power->write(false);
	vTaskDelay(2);
}
extern "C" {
void RIT_IRQHandler(void){

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	if(RIT_count > 0) {
		RIT_count--;
		if( RIT_count % 2 == 0){
			power->write(true);
		}else{
			power->write(false);
		}

	}
	else {
		Chip_RIT_Disable(LPC_RITIMER); // disable timer
		// Give semaphore and set context switch flag if a higher priority task was woken up
		xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}

void RIT_start(int count, int us){
	uint64_t cmp_value;
	clockrate = 1000000 / (clockrate*2);
	// Determine approximate compare value based on clock rate and passed interval
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
	// disable timer during configuration
	Chip_RIT_Disable(LPC_RITIMER);
	RIT_count = count*2;
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

void analyseInput(std::string input){
	char c;
	c = input.at(0);
	if(c == 'l'){
		dir->write(true);//left
		steps = std::stoi (input, nullptr, 16);
	}else if(c == 'r'){
		dir->write(false);//right
		steps = std::stoi (input, nullptr, 16);
	}else if(c == 'p'){
		clockrate = std::stoi (input, nullptr, 16);;
	}

}


SemaphoreHandle_t mutex = NULL;

//read input
static void vTask1(void *pvParameters){

	std::stringstream input_s;
	int input_i;
	char input_c;

	rightSW = new DigitalIoPin(0, 28, DigitalIoPin::pullup, true);		//Boundary1 at motor
	leftSW = new DigitalIoPin(0, 27, DigitalIoPin::pullup, true);		//Boundary2 far from motor
	dir = new DigitalIoPin(1, 0, DigitalIoPin::output, true);			//DIR
	power = new DigitalIoPin(0, 24, DigitalIoPin::output, false); 		//STEPP

	SW1 = new DigitalIoPin(0, 17, DigitalIoPin::pullup, true);
	SW2 = new DigitalIoPin(1, 11, DigitalIoPin::pullup, true);
	SW3 = new DigitalIoPin(1, 9, DigitalIoPin::pullup, true);


	while(1){
		vTaskDelay(50);
		input_i = Board_UARTGetChar();
		Board_LED_Set(2, false);
		if(input_i == 10 || input_i == 13){
			Board_UARTPutSTR("Enter has been pressed.\r\n");
			input_s << "\r\n";
			Board_UARTPutSTR(input_s.str().c_str());
			Board_LED_Set(2, true);
			//analyseInput(input_s.str());
				char c;
				std::string string;
				string = input_s.str();
				c = string.at(5);
				if(c == 'l'){
					dir->write(true);//left
					steps = std::stoi(input_s.str(), nullptr, 10);
				}else if(c == 'r'){
					dir->write(false);//right
					steps = std::stoi(string, nullptr, 10);
				}else if(c == 'p'){
					clockrate = std::stoi(input_s.str(), nullptr, 10);
				}

			Board_LED_Set(1, true);
			RIT_start(steps, clockrate);
			Board_LED_Set(2, false);
			input_s.str("");
			Board_UARTPutSTR("Success.\r\n");
		}else if((input_i >= 48 && input_i <= 57) || (input_i >= 97 && input_i <= 122)){
				input_c = input_i;
				input_s << input_c;
				vTaskDelay(10);
		}



	}//while(1)
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}
/* end runtime statictics collection */

/**
 * @brief	lab6, semaphores
 * @return	Nothing, function should not exit
 */
int main(void)
{
	prvSetupHardware();
	ITM_init();
	sbRIT = xSemaphoreCreateBinary();

	rightSW = new DigitalIoPin(0, 28, DigitalIoPin::pullup, true);		//Boundary1 at motor
	leftSW = new DigitalIoPin(0, 27, DigitalIoPin::pullup, true);		//Boundary2 far from motor
	dir = new DigitalIoPin(1, 0, DigitalIoPin::output, true);			//DIR
	power = new DigitalIoPin(0, 24, DigitalIoPin::output, false); 		//STEPP

	SW1 = new DigitalIoPin(0, 17, DigitalIoPin::pullup, true);
	SW2 = new DigitalIoPin(1, 11, DigitalIoPin::pullup, true);
	SW3 = new DigitalIoPin(1, 9, DigitalIoPin::pullup, true);

	xTaskCreate(vTask1, "vTask1",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();
	/* Should never arrive here */
	return 1;
}
