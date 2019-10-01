/*
===============================================================================
 Name        : main.c
 Author      : $lpoeppel
 Version     : lab2ex3
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



#include <cr_section_macros.h>

// TODO: insert other include files here
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// TODO: insert other definitions and declarations here

#include <string>

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
	/* Initial greenLED0, redLED1, blueLED2 state is off */
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);

}

SemaphoreHandle_t binary = NULL;
SemaphoreHandle_t mutex = NULL;
SemaphoreHandle_t mutexWrite = NULL;

void chooseOutput(){
	int i = std::rand() % 5;
	if (i < 0) {i = -i;}
	i+= 1;

	switch(i){
		case 1: Board_UARTPutSTR("[Oracle] A very good question indeed. \r\n");;
			break;
		case 2: Board_UARTPutSTR("[Oracle] I will have to think about that. \r\n");;
			break;
		case 3: Board_UARTPutSTR("[Oracle] You have too much free time on your hands. \r\n");;
			break;
		case 4: Board_UARTPutSTR("[Oracle] Do you want me to answer that, or shall I just listen while you figure it out yourself. \r\n");;
			break;
		case 5: Board_UARTPutSTR("[Oracle] The future will bring you the answer. \r\n");;
			break;
		default: Board_UARTPutSTR("[Oracle] This seems interesting. \r\n");;
			break;
	}
}

static void vReadTask(void *pvParameters) {
	std::string input_s = "[You] ";
	char c = 'a';
	int input_i = 0;
	bool value = false;
	while(1){
		vTaskDelay(10);
		input_i = Board_UARTGetChar();
		if(input_i != EOF){
			Board_LED_Set(2, true);
			if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE){
				if(input_i == 13 || input_i == 10 || input_s.size() >= 60){
					Board_LED_Set(1, true);
					input_s.append("\r\n");
					Board_UARTPutSTR(input_s.c_str());
					input_s.clear();
					input_s.append("[You] ");
					Board_LED_Set(1, false);
					if(value){
						value = false;
						xSemaphoreGive(binary);
					}
				}else{
					if(input_i == 63){
						value = true;
					}
					c = input_i;
					input_s.push_back(c);
				}//else
				Board_LED_Set(2, false);
			}//if pdTRUE
		}else{
			xSemaphoreGive(mutex);
		}

	}//while (1)
}//vReadTask

static void vOracleTask(void *pvParameters){
	while(1){
		if(xSemaphoreTake(binary, portMAX_DELAY) == pdTRUE){
			Board_LED_Set(0, false);
			Board_LED_Set(1, false);

			Board_LED_Set(2, true);
			if(xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE){
				Board_UARTPutSTR("[Oracle] Hmmmm....\r\n");
				xSemaphoreGive(mutex);
				vTaskDelay(3000);
				xSemaphoreTake(mutex, portMAX_DELAY);
				chooseOutput();
				xSemaphoreGive(mutex);
				vTaskDelay(2000);
				Board_LED_Set(2, false);
			}
		}else{
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
	binary = xSemaphoreCreateBinary();
	mutex = xSemaphoreCreateMutex();
	mutexWrite = xSemaphoreCreateMutex();

	xTaskCreate(vReadTask, "vTaskRead",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);
	xTaskCreate(vOracleTask, "vTaskOracle",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);
	/* Start the scheduler */
	vTaskStartScheduler();
	/* Should never arrive here */
	return 1;
}
