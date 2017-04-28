#include "TM4C123GE6PM.h"
//#include "font8x8.h"
//#include "ADC_PWM.c"
#include "LCD.h"
#include "main.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#define NVIC_EN0_R              (*((volatile unsigned long *)0xE000E100)) // taken from 'TM4C123GE6PM.h'

// ------------------Variable  Definitions -------------
//touch screen variables//
int tempX1;				//First X coord
int tempX2;				//Second X coord
int Xval;				//Final X
int tempY1;				//First Y coord
int tempY2;				//Second Y coord
int Yval;				//Final Y
int garbage;			//Will hold unwanted values
int Pcount = 0;			//used for a counter
bool change = false;	//Interrupt has been triggered
unsigned const char zero = 200; //48
int temp = 0;
//void lines(unsigned short);
//void background(void);
int k =  0;
// ------------------*************CORE PROGRAM ****************-------------
int main(void)
{
	//###############initalize#################
	setup_sysclk();
	setupGPIOD();
	setupUART2();
	connect_BT();
	//connect_BT(void)
	
	
	//setup_PWMA6();
	//setup_ADC();
	setup_SSIO(); 	//setup SPI on PA2-PA5
	setup_PB();     //Setup D/C on PB0
	LCD_Init();		//setup LCD screen
	Touch_init();	//setup touch screen
	flash_screen(white);
	
	lines(black);
	background();
	setup_timer();
    while (1)
    {
    }
}

// ------------------Function Definitions -------------
void setup_sysclk(){	//initialize PLL as system clk source
	SYSCTL->RCC 		|= 0x800; 				//bypass enabled	
	SYSCTL->RCC 		= 0x2400D40; 			//sets PLL to 40MHz
	while (!(SYSCTL->RIS & 0x40)){}				//wait for PLL to stabilize	
	SYSCTL->RCC 		&= ~0x800;				//clear bypass so PLL is used
}
void setup_SSIO(){		
	SYSCTL->RCGCSSI 	|= 0x1;
	SYSCTL->RCGCGPIO 	|= 0x1B;//enable port A, B, and E
	GPIOA->AFSEL 		 	|= 0x3C;
	GPIOA->DEN 			 	|= 0x3C; //Pa2,3,4,5
	GPIOB->DEN 			 	|= 0x1;
	SSI0->CR1 =0x0;		///SSE bit clear in SSICR1
	SSI0->CC = 0x0;		//Configure the SSI clock source
	SSI0->CPSR = 0x2; //Configure the clock prescale divisor
	SSI0->CR0 = 0x07;	//SCR = 0, SPH = 0, SPO = 0, protocol = 0, datasize = 0x7 (8bit)
	SSI0->CR1 =0x2;		//Enable the SSI by 
}

void setup_PB(){   		//Setup Data/Command line and chip select
	GPIOB->AFSEL = 0x0;
	GPIOB->DIR   = 0x7;
	GPIOB->PUR   = 0x7; //PUR
	GPIOB->DEN   = 0x7;	//DEN
	GPIOB->DATA  = 0x5;	//Either 1 or 2
}

void Touch_init(void){	//Setup interrupt for the touch screen
	NVIC_EN0_R 		|= 0x10;	//interupt enable GPIOE(0);
	GPIOE->AFSEL 	= 0x0;
	GPIOE->DIR 		= 0x0;
	GPIOE->PUR 		= 0x1;
	GPIOE->DEN 		= 0x1;
	GPIOE->IS 		= 0x0;
	GPIOE->IBE 		= 0x0;
	GPIOE->IEV 		= 0x0;
	GPIOE->IM		= 0x1;
}

void GPIOPortE_Handler(void){			//Actions from interrupt
	GPIOE->ICR  = 0x1;		//reset the interupt
	GPIOB->DATA = 0x3;		//Forcing TCS low and LCD high
	
	//reset the SSI0 clock to 2MHz
	SSI0->CR1 &= ~0x2;
	SSI0->CPSR = 0x14;
	SSI0->CR1 |= 0x2;
	
	while ((GPIOE->DATA == 0x0) && (Pcount < 2000)){
		while((SSI0->SR & 0x4) == 0x4){
		garbage = SSI0->DR;				//capture data
		}
		/*--------------READ IN X ----------------------*/
		SSI0->DR = 0xD0;				//read in first X cordinate
		while((SSI0->SR & 0x4) == 0x0){	//wait for FIFO to FULL
		};
		garbage = SSI0->DR;  			//Grab garbage Data
		SSI0->DR = 0x00; 				//this is to keep clock going.
		while((SSI0->SR & 0x4) == 0x0){	//wait for FIFO to FULL	
		};
		tempX1 = SSI0->DR;				//CMD to read second X coordinate
		SSI0->DR = 0x00; 				//this is to keep clock going.
		while((SSI0->SR & 0x4) == 0x0){	//wait for FIFO to FULL	
		};
		tempX2 = SSI0->DR;				//Read second coordinate
		/*--------------READ IN Y ----------------------*/
		SSI0->DR = 0x90;				//read in first X cordinate
		while((SSI0->SR & 0x4) == 0x0){	//wait for FIFO to FULL
		};
		garbage = SSI0->DR;  			//Grab garbage Data
		SSI0->DR = 0x00; 				//this is to keep clock going.
		while((SSI0->SR & 0x4) == 0x0){	//wait for FIFO to FULL	
		};
		tempY1 = SSI0->DR;				//CMD to read second X coordinate
		SSI0->DR = 0x00; 				//this is to keep clock going.
		while((SSI0->SR & 0x4) == 0x0){	//wait for FIFO to FULL	
		};
		tempY2 = SSI0->DR;				//Read second coordinate
		Pcount++;	
		Yval = Yval + (tempY1<<4) + (tempY2>>4); //Yval = mask of tempY1 and tempY2	
		Xval = Xval + (tempX1<<4) + (tempX2>>4); //Xval = mask of tempX1 and tempX2
		}
	/*-------------------------------------------------*/
	Yval = (Yval / Pcount);
	Xval = (Xval / Pcount);
	Pcount = 0;
	/*-------------------------------------------------*/
	//Reset SSIO clock back to 20MHz	
	SSI0->CR1 &= ~0x2;
	SSI0->CPSR = 0x2;
	SSI0->CR1 |= 0x2;
	
	GPIOB->DATA = 0x5;	//Forcing TCS high and LCD low
	change = true;	
	Yval = 0;
	Xval = 0;
	while (GPIOE->DATA == 0x0){//debouncer function
	}	
}
void background(){
	
	writeLetter(7,40,5); //Write space
	writeLetter(6,40,19); //Write a 5
	//
	writeLetter(0,90,5); //Write a 1
	writeLetter(7,90,19); //Write a 0
			
	//
	writeLetter(0,135,5); //Write a 1
	writeLetter(6,135,19); //Write a 5
	//
	writeLetter(8,185,5); //Write a 2
	writeLetter(7,185,19); //Write a 0
	//
	writeLetter(10,5,380); //Write a T
	writeLetter(11,5,394); //Write a I
	writeLetter(12,5,408); //Write a M
	writeLetter(13,5,422); //Write a E
	
}

void lines(unsigned short color){	//Draw Gridlines
	int p;
	int Sx, Fx, Dx;
	int Sy, Fy, Dy;
	//x position variables.
	Sx = 48;
	Fx = 47;
	Dx = abs(Fx - Sx);
	//y position variables.
	Sy = 30;
	Fy = 830;
	Dy = abs(Fy - Sy);
	
	//set column
	writeCmd(0x2A);			//Write to X axis
	writeDat2(Sx); 			//start x 161
	writeDat2(Fx);			//end x 230
	//set page
	writeCmd(0x2B); 		//write to Y axis
	writeDat2(Sy);  		//start y
	writeDat2(Fy);  		//end y
	
	writeCmd(0x2C);
	for(p = 0; p < Dx*Dy; p++)
		writeDat2(color);
	//---------------------//
	Sx = 96;
	Fx = 95;
	Dx = abs(Fx - Sx);
	//y position variables.
	Sy = 30;
	Fy = 830;
	Dy = abs(Fy - Sy);
	
	//set column
	writeCmd(0x2A);			//Write to X axis
	writeDat2(Sx); 			//start x 161
	writeDat2(Fx);			//end x 230
	//set page
	writeCmd(0x2B); 		//write to Y axis
	writeDat2(Sy);  		//start y
	writeDat2(Fy);  		//end y
	
	writeCmd(0x2C);
	for(p = 0; p < Dx*Dy; p++)
		writeDat2(color);
			//---------------------//
	Sx = 144;
	Fx = 143;
	Dx = abs(Fx - Sx);
	//y position variables.
	Sy = 30;
	Fy = 830;
	Dy = abs(Fy - Sy);
	
	//set column
	writeCmd(0x2A);			//Write to X axis
	writeDat2(Sx); 			//start x 161
	writeDat2(Fx);			//end x 230
	//set page
	writeCmd(0x2B); 		//write to Y axis
	writeDat2(Sy);  		//start y
	writeDat2(Fy);  		//end y
	
	writeCmd(0x2C);
	for(p = 0; p < Dx*Dy; p++)
		writeDat2(color);
			//---------------------//
	Sx = 192;
	Fx = 191;
	Dx = abs(Fx - Sx);
	//y position variables.
	Sy = 30;
	Fy = 830;
	Dy = abs(Fy - Sy);
	
	//set column
	writeCmd(0x2A);			//Write to X axis
	writeDat2(Sx); 			//start x 161
	writeDat2(Fx);			//end x 230
	//set page
	writeCmd(0x2B); 		//write to Y axis
	writeDat2(Sy);  		//start y
	writeDat2(Fy);  		//end y
	
	writeCmd(0x2C);
	for(p = 0; p < Dx*Dy; p++){
		writeDat2(color);

	}
}




