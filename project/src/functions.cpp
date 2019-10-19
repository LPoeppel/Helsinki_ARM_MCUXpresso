/*
 * functions.cpp
 *
 *  Created on: 16.10.2019
 *      Author: Lukas
 */

#include "functions.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "user_vcom.h"
#include "ITM_write.h"
#include <string>

/***************************************/
/***************************************/

Motor *MotorX, *MotorY;
ServoMotor *MotorPen, *MotorLaser;
config plotter = {.type = "XY", .height=LENGTHY, .width=LENGTHX,
					.xDir=XDIRORIGIN, .yDir=YDIRORIGIN};

/***************************************/
/***************************************/


void mirror(int input){
	char ch[2];
	ch[0] = input;
	ch[1] = 0;
	ITM_write(ch);
}

int isNotEmpty(char *input){
	size_t length = sizeof(*input)/sizeof(input);
	if(length >= 0 && length <= 64){
		return 1;
	}else{
		return 0;
	}
}

void accelerate(int x0, int x1, int y0, int y1){
	int dx = abs(x1-x0);
	int dy = abs(y1-y0);
	int D2 = (dx*dx)+(dy*dy);

	if(D2 > 200){
		if(((MotorX->getSpeed())+10) < 100){
			MotorX->setSpeed((MotorX->getSpeed())+10);
		}else{
			MotorX->setSpeed(100);
		}
		if(((MotorY->getSpeed())+10) < 100){
			MotorY->setSpeed((MotorY->getSpeed())+10);
		}else{
			MotorY->setSpeed(100);
		}

	}else{
		MotorX->setSpeed(0);
		MotorY->setSpeed(0);
	}

}
void moveTo(int zero){
	int x1 = 0, y1 = 0;
	int x0 = MotorX->getPos();
	int y0 = MotorX->getPos();
	MotorX->setDir(XDIRORIGIN);
	MotorY->setDir(YDIRORIGIN);
	int dx = x0;						//distance X-axis
	int dy = y0;						//distance Y-axis

//	int D2 = (dx*dx)+(dy*dy);			//actual distance / hypothenuse
//	int px = 2*dy-dx;					//slope x
//	int py = 2*dx-dy;					//slope y
	int ex = 0, ey = 0;					//

	if(dx >= dy){						//X-axis dominant
		for(int i = 0; i < dx; i++){
			accelerate(x0, x1, y0, y1);
			MotorX->move(1);
			ey+=dx;
			if(ey >= dx){
				ey-=dx;
				MotorY->move(1);
			}
		}
	}else{								//Y-axis dominant
		for(int i = 0; i < dy; i++){
			accelerate(x0, x1, y0, y1);
			MotorY->move(1);
			ex+=dy;
			if(ex >= dy){
				ex-=dy;
				MotorY->move(1);
			}
		}
	}
}
//Bresenhams algorithm
void moveTo(int x1, int y1, bool relative){
	int x0 = MotorX->getPos();
	int y0 = MotorX->getPos();
	(x1<x0) ? MotorX->setDir(XDIRORIGIN) : MotorX->setDir(!XDIRORIGIN);
	(y1<y0) ? MotorY->setDir(YDIRORIGIN) : MotorY->setDir(!YDIRORIGIN);

	int dx = abs(x1-x0);				//distance X-axis
	int dy = abs(y1-y0);				//distance Y-axis
//	int D2 = (dx*dx)+(dy*dy);			//actual distance / hypothenuse
//	int px = 2*dy-dx;					//slope x
//	int py = 2*dx-dy;					//slope y
	int ex = 0, ey = 0;					//

	if(dx >= dy){						//X-axis dominant
		for(int i = 0; i < dx; i++){
			accelerate(x0, x1, y0, y1);
			MotorX->move(1);
			ey+=dx;
			if(ey >= dx){
				ey-=dx;
				MotorY->move(1);
			}
		}
	}else{								//Y-axis dominant
		for(int i = 0; i < dy; i++){
			accelerate(x0, x1, y0, y1);
			MotorY->move(1);
			ex+=dy;
			if(ex >= dy){
				ex-=dy;
				MotorY->move(1);
			}
		}
	}

}

void sendM10(){
	std::string input;
	input.append("M10 XY ");
	input.append(std::to_string(plotter.height));
	input.push_back(' ');
	input.append(std::to_string(plotter.width));
	input.append(" 0.00 0.00 A");
	input.append(std::to_string(plotter.xDir));
	input.append(" B");
	input.append(std::to_string(plotter.yDir));
	input.append(" H0 S");
	input.append(std::to_string(plotter.plottingSpeed));
	input.append(" U");
	input.append(std::to_string(plotter.penUp));
	input.append(" D");
	input.append(std::to_string(plotter.penDown));
	char c = (int) 13;
	input.push_back(c);
	const unsigned char *constStr = reinterpret_cast<const unsigned char *>(input.c_str());
	USB_send((uint8_t*) constStr, input.size()+1);
}
void sendM11(int L4, int L3, int L2, int L1){
	std::string input;
	input.append("M10 ");
	input.append(std::to_string(L4));
	input.push_back(' ');
	input.append(std::to_string(L3));
	input.push_back(' ');
	input.append(std::to_string(L2));
	input.push_back(' ');
	input.append(std::to_string(L1));
	char c = (int) 13;
	input.push_back(c);
	const unsigned char *constStr = reinterpret_cast<const unsigned char *>(input.c_str());
	USB_send((uint8_t*) constStr, input.size()+1);

}

// Sets up system hardware
void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	ITM_init();
	Chip_RIT_Init(LPC_RITIMER);
	NVIC_SetPriority(RITIMER_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY +1);

//	 Initial greenLED0, redLED1, blueLED2 state is off
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);

	MotorX = new Motor(new DigitalIoPin(0, 27, DigitalIoPin::pullup, true),
							new DigitalIoPin(0, 28, DigitalIoPin::pullup, true),
							new DigitalIoPin(0, 0, DigitalIoPin::pullup, true),
							new DigitalIoPin(1, 3, DigitalIoPin::pullup, true),
							LENGTHX);
	MotorY = new Motor(new DigitalIoPin(0, 24, DigitalIoPin::pullup, true),
							new DigitalIoPin(1, 0, DigitalIoPin::pullup, true),
							new DigitalIoPin(0, 29, DigitalIoPin::pullup, true),
							new DigitalIoPin(0, 9, DigitalIoPin::pullup, true),
							LENGTHY);
	MotorPen = new ServoMotor(0, 10, PENFRQ, PENCYCLE);
	MotorLaser = new ServoMotor(0, 12, 255);
//	MotorX->calibrate();
//	MotorY->calibrate();

}


command_struct parse(QueueHandle_t &xQueueCommands){
//	xSemaphoreTake(mutex, portMAX_DELAY);
	command_struct cmd;
	xQueueReceive(xQueueCommands, cmd.input, 10);
	char *cmdlet;
	cmdlet = strtok(cmd.input," ");

	if(!strncmp(cmdlet, "G1", 2)){
		cmd.type = G1;
	}else if(!strncmp(cmdlet,"G28", 3)){
		cmd.type = G28;
	}else if(!strncmp(cmdlet, "M10", 3)){
		cmd.type = M10;
	}else if(!strncmp(cmdlet, "M11", 3)){
		cmd.type = M11;
	}else if(!strncmp(cmdlet, "M1", 2)){
		cmd.type = M1;
	}else if(!strncmp(cmdlet, "M2", 2)){
		cmd.type = M2;
	}else if(!strncmp(cmdlet, "M4", 2)){
		cmd.type = M4;
	}else if(!strncmp(cmdlet, "M5", 2)){
		cmd.type = M5;
	}else{
		cmd.type = Z0;
	}

	return cmd;
}//parse()


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
void run(command_struct cmd){
	int counter = strlen(cmd.input);
	int coloumns = 0;
	bool trigger = true;			//only creating array for commands if needed

	switch(cmd.type){
		case G1:
			if(counter == 3){coloumns = counter;}
			break;
		case G28:
			trigger = false;
			moveTo(0);
			break;
		case M1:
			if(counter == 1){coloumns = counter;}
			break;
		case M2:
			if(counter == 2){coloumns = counter;}
			break;
		case M4:
			if(counter == 1){coloumns = counter;}
			break;
		case M5:
			if(counter == 5){coloumns = counter;}
			break;
		case M10:
			trigger = false;
			sendM10();
			break;
		case M11:
			if(counter == 4){coloumns = counter;}
			break;
	}//switch

	ITM_write("Functions have been called.");
	if(trigger){
		char **array = new char*[8];
		char *cmdlet = strtok(cmd.input," ");
		for(int i = 0; cmdlet != NULL; i++){
			cmdlet = strtok(NULL," ");
			array[i] = cmdlet;
		}

		switch(cmd.type){
				case G1:
				{
					float valueX = (atof(array[0]+1)*10);
					float valueY = (atof(array[1]+1)*10);
					moveTo((int) valueX, (int) valueY, atoi(array[2]+1));
				}
					break;
				//case 28 does not need to be covered, see switch above
				case M1:
					MotorPen->move(atoi(array[0]));
					break;
				case M2:
					plotter.penUp = atoi(array[0]+1);
					plotter.penDown = atoi(array[1]+1);
					break;
				case M4:
//					MotorLaser->laser(atoi(array[0]));
					break;
				case M5:
					plotter.xDir = atoi(array[0]+1);
					plotter.yDir = atoi(array[1]+1);
					plotter.height = atoi(array[2]+1);
					plotter.width = atoi(array[3]+1);
					plotter.plottingSpeed = atoi(array[4]+1);
					break;
				//case M10 does not to be covered, see switch above
				case M11:
				{
					int L4 = !(MotorX->readMaxLSW());
					int L3 = !(MotorX->readMinLSW());
					int L2 = !(MotorY->readMaxLSW());
					int L1 = !(MotorY->readMinLSW());
					sendM11(L4, L3, L2, L1);
				}
					break;
			}//switch

		for(int i = 0; i < coloumns; i++) {
				delete[] array[i];
			}
		delete[] array;
	}//trigger
}//run()
#pragma GCC diagnostic pop


