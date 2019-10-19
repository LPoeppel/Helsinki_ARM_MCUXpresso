/*
===============================================================================
 Name        : main.c
 Author      :
 Version     : project
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
#include "ITM_write.h"
#include "user_vcom.h"

// TODO: insert other definitions and declarations here

#include "config.h"
#include "functions.h"

//
//  Private functions
//

QueueHandle_t xQueueCommands = NULL;
TaskHandle_t xTaskMain = NULL;
TaskHandle_t xTaskRead = NULL;

//Tasks
static void vMainTask(void *pvParameters){
	vTaskDelay(100);
	ITM_write("online MainTask\r\n");
	unsigned char ok[] = "OK\n";
	while(1){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		command_struct type = parse(xQueueCommands);
		if(type.type != Z0){
			run(type);
			ITM_write("OK\r\n");
			//xSemaphoreGive(mutex);
//			xTaskNotifyGive(xTaskRead);
			USB_send(ok, 3);
		}else{
			ITM_write("FAIL\n\r");
			vTaskSuspend(NULL);
		}
	}//while(1)
}

 static void vReadTask( void *pvParameters ){
	vTaskDelay(100);

	ITM_write("online ReadTask\r\n");
	char buffer[64];

	while(1){
//		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		USB_receive((uint8_t *)buffer, 64);
		if(!isNotEmpty(buffer)){
			vTaskDelay(10);
		}else{
			xQueueSendToBack(xQueueCommands, buffer, 10);
			xTaskNotifyGive(xTaskMain);
		}
	}//while(1)

}

// the following is required if runtime statistics are to be collected
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}

//
//  @brief	project, plotter
//  @return	Nothing, function should not exit
//
int main(void)
{

	prvSetupHardware();

	xQueueCommands = xQueueCreate(5, sizeof(char)*64);

	xTaskCreate(vReadTask, "vTaskRead",
					configMINIMAL_STACK_SIZE + 512, NULL, (tskIDLE_PRIORITY + 1UL),
						&xTaskRead);
	xTaskCreate(vMainTask, "vTaskMain",
						configMINIMAL_STACK_SIZE + 1024, NULL, (tskIDLE_PRIORITY + 1UL),
						&xTaskMain);
	xTaskCreate(cdc_task, "CDC",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

//	 Start the scheduler
	vTaskStartScheduler();
//	 Should never arrive here
	return 1;
}
