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

#include <string>
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
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
}

//Reads from UART input. If <LF> it prints OK,
//holding the mutex for the print process and
//instantly releasing it
SemaphoreHandle_t mutex = NULL;
static void vReadTask(void *pvParameters) {
	int input_i;
	while(1){
		input_i = Board_UARTGetChar();
		if(input_i != EOF){
			if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE){
				if(input_i == 13){ //ASCII 13 = <LF>
					Board_UARTPutSTR("OK\r\n");
				}
			}
			xSemaphoreGive(mutex);
		}
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
 * @brief	lab2, semaphores
 * @return	Nothing, function should not exit
 */
int main(void)
{
	prvSetupHardware();
	mutex = xSemaphoreCreateMutex();
	xTaskCreate(vReadTask, "vTaskRead",
		configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
		(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();
	/* Should never arrive here */
	return 1;
}
