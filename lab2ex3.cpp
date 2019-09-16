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
#include <string>
#include <sstream>
#include <ostream>
#include <cstring>
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

SemaphoreHandle_t binary = NULL;
SemaphoreHandle_t counting = NULL;
TaskHandle_t xOracleTask = NULL;
static void vReadTask(void *pvParameters) {
	std::stringstream output;
	output << "[YOU] ";
	std::string input;
	char c;
	while(1){
		int inputc = Board_UARTGetChar();
		if(inputc != EOF){
			if(xSemaphoreTake(binary, portMAX_DELAY) == pdTRUE){
				if(inputc == 13 || inputc == 10 || input.size() == 60){
							output << input << "\n";
							Board_UARTPutSTR(output.str().c_str());
							xSemaphoreGive(binary);
						}
				else if(inputc != 13 && inputc != 10){
					if(inputc == 63){ xTaskNotify(xOracleTask, 1, eIncrement); }
					c = inputc;
					input.append(&c);
				}
			}else{
				xSemaphoreGive(binary);
			}
		}
		else{ vTaskDelay(50); }
	}
}

static void vOracleTask(void *pvParameters){
	while(1){
		if(xSemaphoreTake(binary, portMAX_DELAY) == pdTRUE){
			if( xTaskGetHandle("vOracleIndicate") ){
				Board_UARTPutSTR("[Oracle] Hmmm…");

				xSemaphoreGive(binary);
				vTaskDelay(3000);
				if(xSemaphoreTake(binary, portMAX_DELAY) == pdTRUE){
					std::srand(3);
					int i = std::rand();
					std::string output = "[Oracle] ";
					switch(i){
						case 0: output.append("This seems like a difficult question...");
							break;
						case 1: output.append("You should think about the problem from a different perspective.");
							break;
						case 2: output.append("Yes!");
							break;
						case 3: output.append("No.");
							break;
						default: output.append("This should be thoroughly thought thru..");
							break;
					}
					Board_UARTPutSTR(output.c_str());
				}
				xSemaphoreGive(binary);
				vTaskDelay(2000);
			}
		}else{
			xSemaphoreGive(binary);
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
	counting = xSemaphoreCreateCounting(5, 0);

	xTaskCreate(vReadTask, "vTaskIndicatRead",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					(TaskHandle_t *) NULL);
	xTaskCreate(vOracleTask, "vOracleIndicate",
					configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
					&xOracleTask);
	/* Start the scheduler */
	vTaskStartScheduler();
	/* Should never arrive here */
	return 1;
}
