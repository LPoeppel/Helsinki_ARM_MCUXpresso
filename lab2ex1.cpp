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
}
//Lab2 Ex1
SemaphoreHandle_t mutex = NULL;
static void vMutexTask(void *pvParameters) {
			mutex = xSemaphoreCreateMutex();
			xSemaphoreGive(mutex);
			while(1){
				;
			}
}
/* UART (or output) thread */
static void vUARTTask1(void *pvParameters) {
		DigitalIoPin SW1(0, 17, DigitalIoPin::pullup, true);
		while (1) {
			if(SW1.read()){
				if(mutex != NULL){
					if(xSemaphoreTake(mutex, ( TickType_t ) 10) == pdTRUE)
					{
						while(SW1.read()){
							Board_UARTPutSTR("SW1 is pressed.\n\n");
							vTaskDelay(configTICK_RATE_HZ);
						}
						xSemaphoreGive(mutex);
					}
				}
			}
		}
}

static void vUARTTask2(void *pvParameters) {
	DigitalIoPin SW2(1, 11, DigitalIoPin::pullup, true);
	while (1) {
		if(SW2.read()){
			if(mutex != NULL){
				if(xSemaphoreTake(mutex, ( TickType_t ) 10) == pdTRUE)
				{
					while(SW2.read()){
						Board_UARTPutSTR("SW2 is pressed.\n\n");
						vTaskDelay(configTICK_RATE_HZ);
					}
					xSemaphoreGive(mutex);
				}
			}
		}
	}
}
static void vUARTTask3(void *pvParameters) {
	DigitalIoPin SW3(1, 9, DigitalIoPin::pullup, true);
	while (1) {
		if(SW3.read()){
			if(mutex != NULL){
				/* See if we can obtain the semaphore.  If the semaphore is not
				available wait 10 ticks to see if it becomes free. */
				if(xSemaphoreTake(mutex, ( TickType_t ) 10) == pdTRUE)
				{
					while(SW3.read()){
						Board_UARTPutSTR("SW3 is pressed.\n\n");
						vTaskDelay(configTICK_RATE_HZ);
					}
					xSemaphoreGive(mutex);
				}
			}
		}
	}
}
//Lab2 Ex2
static void vReadUARTTask(void *pvParameters) {
	DigitalIoPin SW3(1, 9, DigitalIoPin::pullup, true);
	while (1) {
		if(SW3.read()){
			if(mutex != NULL){
				/* See if we can obtain the semaphore.  If the semaphore is not
				available wait 10 ticks to see if it becomes free. */
				if(xSemaphoreTake(mutex, ( TickType_t ) 10) == pdTRUE)
				{
					while(SW3.read()){
						Board_UARTPutSTR("SW3 is pressed.\n\n");
						vTaskDelay(configTICK_RATE_HZ);
					}
					xSemaphoreGive(mutex);
				}
			}
		}
	}
}
static void vIndicateReadTask(void *pvParameters) {
	DigitalIoPin SW3(1, 9, DigitalIoPin::pullup, true);
	while (1) {
		if(SW3.read()){
			if(mutex != NULL){
				/* See if we can obtain the semaphore.  If the semaphore is not
				available wait 10 ticks to see if it becomes free. */
				if(xSemaphoreTake(mutex, ( TickType_t ) 10) == pdTRUE)
				{
					while(SW3.read()){
						Board_UARTPutSTR("SW3 is pressed.\n\n");
						vTaskDelay(configTICK_RATE_HZ);
					}
					xSemaphoreGive(mutex);
				}
			}
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

	//Lab2 Ex1
	
	xTaskCreate(vMutexTask, "vTaskMutex",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);
	xTaskCreate(vUARTTask1, "vTaskUart1",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);
	xTaskCreate(vUARTTask2, "vTaskUart2",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);
	xTaskCreate(vUARTTask3, "vTaskUart3",
				configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();
	/* Should never arrive here */
	return 1;
}
