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
//Lab2 Ex2
SemaphoreHandle_t binary = NULL;
static void vReadTask(void *pvParameters) {
	while (1) {
		int i = Board_UARTGetChar();
		if(i != EOF){
			xSemaphoreGive(binary);
			Board_UARTPutChar(i);
		}
		else{ vTaskDelay(1); }
	}
}
static void vIndicateTask(void *pvParameters) {
	while (1) {
		if(xSemaphoreTake(binary, portMAX_DELAY) == pdTRUE){
			Board_LED_Toggle(1);
			vTaskDelay(configTICK_RATE_HZ * 0.1);
			Board_LED_Toggle(1);
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
	binary = xSemaphoreCreateBinary();
	//Lab2 Ex2
	xTaskCreate(vReadTask, "vTaskRead",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);
	xTaskCreate(vIndicateTask, "vTaskIndicate",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();
	/* Should never arrive here */
	return 1;
}
