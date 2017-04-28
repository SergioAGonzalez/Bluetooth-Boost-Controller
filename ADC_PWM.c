#include "TM4C123GE6PM.h"
#include "LCD.h"

void setup_timer(void);

void enableClock(void);
void setupUART2(void);
void setupGPIOD(void);
void connect_BT(void);
char readChar(void);
void writeChar(char);
void wait(int);
unsigned char UART_InChar(void);
void UART_OutChar(unsigned char);
int ascii_hex( char);

int hardtemp = 0;
int count = 0;
int ADCin = 0;
int ADC = 0;
int ADC_old = 0;
int ADC_current =0;
int ADC_next = 0;
int Ypos = 40;
int Xpos = 0;
int XS = 0;
int XF = 0;
int Yprev = 38;
int Xprev = 0;
int lineWidth = 1;

char c;
char data;
char u_garbage;
char adc_Val[9] = {0,0,0,0,0,0,0,0,0};
int adc_ValConv =0;
char previous;
char current;
//int ADC = 0;
int i = 0;
int j = 0;

int tol = 200;


unsigned char on[] = {'P','Z',' '};
unsigned char on_len = 3;
unsigned int let[] = {80,100,90};
unsigned char let_size = 3;
void setupUART2(void)//used to send out data via TX functionality
{
	//wait(500);
	SYSCTL->RCGCUART = 0x4;
	wait(500);
	UART2->CTL			=0x0;
	UART2->IBRD			=0x15; 		//CALCULATE FOR 115200 brd
	UART2->FBRD			=0x2D; 		//CALCULATE FOR 115200 brd
	UART2->LCRH			=0x00000070;	//parity disabled, 8 bit word length, 
	UART2->CC			=0x0;
	UART2->CTL			=0x301;
}
void setupGPIOD(){ //setup of AIN5 GPIOD(2)
	SYSCTL->RCGCTIMER  |= 0x1;				//clock for timer 0
	SYSCTL->RCGCGPIO   |= 0x8;
	GPIOD->LOCK 	  	= 0x4c4f434b;
	GPIOD->CR			= 0x80;		//New addition for Bluetooth
	GPIOD->AFSEL 		= 0xC0;		//Alternate Function 0x4 original
	GPIOD->DEN 			= 0xC0;		//digital DISABLE
	GPIOD->PCTL			= 0x11000000; //new addition for BT
}
void setup_timer(void){ //timer 1 set to run at 100mS or 1/10 sec
	SYSCTL->RCGCTIMER  |= 0x2;
	TIMER1->CTL 		= 0x0; 		//using timer 1
	TIMER1->CFG 		= 0x0;				
	TIMER1->TAMR 		= 0x22;				
	TIMER1->TAILR 		= 0x150000; //set to 100mS 250 makes 250mS 0xA0900
	TIMER1->TAMATCHR	= 0x0;				
	TIMER1->CTL 		= 0x1;		//start timer1
	TIMER1->IMR 		= 0x10;
	NVIC->ISER[0] 	   |= 0x204000;
}
void connect_BT(void){
	wait(20000000);
	UART_OutChar('E');
	UART_OutChar('\n');
	wait(20000000);
	UART_OutChar('!');
	UART_OutChar(',');
	UART_OutChar('1');
	UART_OutChar('\n');
	wait(20000000);
	UART_OutChar('+');
	UART_OutChar('\n');
	wait(20000000);
	while((UART2->FR)<<4  & 0x0){
			u_garbage = UART_InChar();
		}	
}

void Timer1A_Handler(void){ //100mS

	int p;
	int Sx, Fx, Dx;
	int Sy, Fy, Dy;
	TIMER1->ICR 	=0x10; 			//clear interrupt
	
	UART_OutChar('@');
	UART_OutChar('I');
	UART_OutChar(',');
	UART_OutChar('0');
	UART_OutChar('\n');
	while((UART2->FR & 0x0010) ==0){
		u_garbage = UART_InChar();
	}
	while(i < 9){ 
		data = UART_InChar();	
		adc_Val[i] = data;		
		i++;
	}
	for(i = 6; i < 9; i++){
		adc_ValConv = adc_ValConv + (ascii_hex(adc_Val[i])<<(8-(4*(i-6))));	
	}
		
	ADC = adc_ValConv;
	adc_ValConv = 0;		
	i = 0;	
	Xpos = ADC/5; //12	



if (Xpos > Xprev){      //line headed +
		XF = Xpos ;
		XS = Xprev+1 ;
	}
	else{			//line headed -
		XF = Xprev;
		XS = Xpos+1;
	}
	
	
	if( Xpos > 3 )
	{
		//x position variables. (VERTICAL AXIS)
		Sx = XS-lineWidth;
		Fx = XF;
		Dx = abs(Fx - Sx)+1;
		//y position variables. (HORIZONTAL AXIS)
		Sy = Yprev-lineWidth;
		Fy = Ypos;
		Dy = abs(Sy - Fy)+1;	//Width of the Plot Line
	
		//set column
		writeCmd(0x2A);			//Write to X axis
		writeDat2(Sx); 			//start x
		writeDat2(Fx);			//end x
		//set page
		writeCmd(0x2B); 		//write to Y axis
		writeDat2(Sy);  		//start y
		writeDat2(Fy);  		//end y
	
		writeCmd(0x2C);
		for(p = 0; p < Dx*Dy; p++){
			writeDat2(red);
		}
			Yprev = Ypos;
			Ypos += 2;
			Xprev = Xpos;


		
		
		if (Ypos >= 320){
			Ypos = 40;
			flash_screen(white);
			lines(black);
			//
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
			//
			
			writeSLetter(80,5,20); //P
			writeSLetter(83,15,20); //S
			writeSLetter(73,25,20); //I
			
			
			
			
		}
	}
}
unsigned char UART_InChar(void){
	while((UART2->FR & 0x0010) !=0); //wait until RXFE is 0
	return((unsigned char)(UART2->DR &0xFF));
}
void UART_OutChar(unsigned char data){
	while((UART2->FR & 0x0020) !=0); //wait until TXFF is 0
	UART2->DR = data;
}
void wait(int i)
{
	int count = 0;
	while(count < i)
	{
		count++;
	}
}

char readChar(void)
{
	char c;
	while((UART2->FR & (1<<4)) != 0)
	{
		c = UART2->DR;
	}
	return c;
}

void writeChar(char c)
{
	UART2->DR = c;
	while((UART2->FR & (1<<3)) != 0)
	{
	}
}
int ascii_hex(char letter){
	switch(letter){
		case '0':
			return 0x0;
		case '1':
			return 0x1;
		case '2':
			return 0x2;
		case '3':
			return 0x3;
		case '4':
			return 0x4;
		case '5':
			return 0x5;
		case '6':
			return 0x6;
		case '7':
			return 0x7;
		case '8':
			return 0x8;
		case '9':
			return 0x9;
		case 'A':
			return 0xA;
		case 'B':
			return 0xB;
		case 'C':
			return 0xC;
		case 'D':
			return 0xD;
		case 'E':
			return 0xE;
		case 'F':
			return 0xF;
		default:
			
			break;
	}
	//return 0;
}