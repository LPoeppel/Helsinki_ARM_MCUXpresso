/*
===============================================================================
 Name        : main.c
 Author      : $(author)
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

// TODO: insert other definitions and declarations here

#include "FreeRTOS.h"
#include "task.h"

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
	Board_LED_Set(0, false);
}

/* LED1 red toggle thread */
static void vLEDTask1(void *pvParameters) {
	bool LedState = false;
	while (1) {
		Board_LED_Set(0, LedState);
		LedState = (bool) !LedState;
		// About a 3Hz on/off toggle rate
		vTaskDelay(configTICK_RATE_HZ / 6);
	}
}

/* LED2 green toggle thread */
static void vLEDTask2(void *pvParameters) {
	bool LedState = false;
	while (1) {
		Board_LED_Set(1, LedState);
		LedState = (bool) !LedState;
		// About a 1Hz on/off toggle rate
		vTaskDelay(configTICK_RATE_HZ / 0.1);
	}
}

//SOS Toggling both LEDs
//Frequency on/off toggle rate: (2x Divisor)Hz
static void vLEDTask3(void *pvParameters) {
	bool LedState = false;
	Board_LED_Set(1, LedState);

	while (1) {
		Board_LED_Set(0, LedState);

		for(int i = 0; i < 6; i++){
			Board_LED_Toggle(0);
			vTaskDelay(configTICK_RATE_HZ / 3);
		}

		for(int i = 0; i < 5; i++){
			Board_LED_Toggle(0);
			vTaskDelay(configTICK_RATE_HZ / 1);
		}

		for(int i = 0; i < 6; i++){
			Board_LED_Toggle(0);
			vTaskDelay(configTICK_RATE_HZ / 3);
		}

		Board_LED_Toggle(1);

	}
}

/* UART (or output) thread */
static void vUARTTask(void *pvParameters) {
	int sec=0, min=0;

	while (1) {
		while(sec >= 60){sec=sec-60; min+=1;}
		DEBUGOUT("Tick: %02d\:%02d \r\n", min, sec);

		DigitalIoPin Pin17(0, 17, DigitalIoPin::pullup, true);
		if(!Pin17.read()){ sec++; }
		else if(Pin17.read()){ sec+=10; }

		/* About a 1s delay here */
		vTaskDelay(configTICK_RATE_HZ);
	}
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
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
int main(void)
{
	prvSetupHardware();

/*	// LED1 toggle thread
	xTaskCreate(vLEDTask1, "vTaskLed1",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	// LED2 toggle thread
	xTaskCreate(vLEDTask2, "vTaskLed2",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);
*/
	// SOS toggle thread
	xTaskCreate(vLEDTask3, "vTaskLed3",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);
	/* UART output thread, simply counts seconds */
	xTaskCreate(vUARTTask, "vTaskUart",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

