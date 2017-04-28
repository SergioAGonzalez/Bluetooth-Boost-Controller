#include "LCD.H"
#include "font8x8_basic.h"
#include "BigFont.h"
#include "TM4C123GE6PM.h"

//------------------Function Definitions ---------
void LCD_Init()
{
	int i;
    writeCmd(0xCB);
    writeDat(0x39);
    writeDat(0x2C);
    writeDat(0x00);
    writeDat(0x34);
    writeDat(0x02);

    writeCmd(0xCF);
    writeDat(0x00);
    writeDat(0XC1);
    writeDat(0X30);

    writeCmd(0xE8);
    writeDat(0x85);
    writeDat(0x00);
    writeDat(0x78);

    writeCmd(0xEA);
    writeDat(0x00);
    writeDat(0x00);

    writeCmd(0xED);
    writeDat(0x64);
    writeDat(0x03);
    writeDat(0X12);
    writeDat(0X81);

    writeCmd(0xF7);
    writeDat(0x20);

    writeCmd(0xC0);    //Power control
    writeDat(0x23);    //VRH[5:0]

    writeCmd(0xC1);    //Power control
    writeDat(0x10);    //SAP[2:0];BT[3:0]

    writeCmd(0xC5);    //VCM control
    writeDat(0x3e); 
    writeDat(0x28);

    writeCmd(0xC7);    //VCM control2
    writeDat(0x86);  	 //--

    writeCmd(0x36);    // Memory Access Control
    writeDat(0x48); 	 //C8    

    writeCmd(0x3A);
    writeDat(0x55);

    writeCmd(0xB1);
    writeDat(0x00);
    writeDat(0x18);

    writeCmd(0xB6);    // Display Function Control
    writeDat(0x08);
    writeDat(0x82);
    writeDat(0x27);

    writeCmd(0xF2);    // 3Gamma Function Disable
    writeDat(0x00);

    writeCmd(0x26);    //Gamma curve selected
    writeDat(0x01);

    writeCmd(0xE0);    //Set Gamma
    writeDat(0x0F);
    writeDat(0x31);
    writeDat(0x2B);
    writeDat(0x0C);
    writeDat(0x0E);
    writeDat(0x08);
    writeDat(0x4E);
    writeDat(0xF1);
    writeDat(0x37);
    writeDat(0x07);
    writeDat(0x10);
    writeDat(0x03);
    writeDat(0x0E);
    writeDat(0x09);
    writeDat(0x00);

    writeCmd(0XE1);    //Set Gamma
    writeDat(0x00);
    writeDat(0x0E);
    writeDat(0x14);
    writeDat(0x03);
    writeDat(0x11);
    writeDat(0x07);
    writeDat(0x31);
    writeDat(0xC1);
    writeDat(0x48);
    writeDat(0x08);
    writeDat(0x0F);
    writeDat(0x0C);
    writeDat(0x31);
    writeDat(0x36);
    writeDat(0x0F);

    writeCmd(0x11);    //Exit Sleep
		for( i = 0; i < 20000; i++) { i++;}

    writeCmd(0x29);    //Display on
    //writeCmd(0x2c);

}

void writeCmd(unsigned char CMD)
{
		while((SSI0->SR & 0x1) == 0x0){};
    GPIOB->DATA &= ~0x1; //This needs to be a modify the first bit ONLY
    SSI0->DR = CMD;
}

void writeDat(unsigned char DAT)
{
		while((SSI0->SR & 0x1) == 0x0){};
    GPIOB->DATA |= 0x1;
    SSI0->DR = DAT;
}

void writeDat2(unsigned short DAT)
{
		while((SSI0->SR & 0x1) == 0x0){};
		GPIOB->DATA |= 0x1;
    SSI0->DR = (DAT >> 8) & 0xFF;
		while((SSI0->SR & 0x1) == 0x0){};
    SSI0->DR = DAT & 0xFF;
}

void flash_screen(unsigned short color)
{
	int p,C,P;
	
	C = 240;
	P = 320;
	
	//set column
	writeCmd(0x2A);
	writeDat2(0x0);
	writeDat2(0xEF);
	//set page
	writeCmd(0x2B);
	writeDat2(0x00);
	writeDat2(0x13F);
	
	writeCmd(0x2C);
	for(p = 0; p < C*P; p++)
		writeDat2(color);
}
//-------------------//
void setArea(unsigned short x1, unsigned short x2, unsigned short y1, unsigned short y2){
	//partial area controls rows first param is start row
	writeCmd(0x2A);//set column address
	writeDat(x1 >> 16);
	writeDat(x1);
	writeDat(x2 >> 16);
	writeDat(x2);
	//set page address
	writeCmd(0x2B);
	writeDat(y1>>16);
	writeDat(y1);
	writeDat(y2>>16);
	writeDat(y2);

}void writeColor(unsigned short color) {

writeDat(color>>8);
writeDat(color);
}
void writeRow(unsigned char row) {
	unsigned char count = 0;
	while (count < 8) {
		if ((row & (1<<count)) == 0)
			writeColor(white);
		else
			writeColor(black);
		count++;
	}
}
void writeSLetter(int offset, unsigned short x, unsigned short y)
{
	unsigned char i = 0;
	setArea(x,x+7,y,y+7);
	writeCmd(0x2C);
	while(i<8)
	{
		writeRow(font8x8_basic[offset][i++]); //offsett, i++
	}
}
void writeSmallString(unsigned short x, unsigned short y, unsigned int array[],unsigned int len) {
	unsigned char j=0;
	unsigned short curX,curY;
	curX = x;
	curY = y;
	while (j < len) {
		writeSLetter(array[j],curX,curY);
		curY += 8; //13
		j++;
		}
	curY = 0;
		curX = 0;
}
void writeBigRow(unsigned int row) {
	unsigned int count2 = 0;
	while (count2 < 32) {
	if ((row & (1<<(count2+1))) == 0)
		writeColor(white);
	else{
		writeColor(blue);
	}
	count2++;	
	}
}
void writeLetter(int offset, unsigned short x, unsigned short y)
{
	unsigned int i = 0;
	unsigned char j = 0;
	unsigned char letterColor = 0;
	unsigned char bigChar = offset;
	setArea(x,x+31,y,y+13);
	writeCmd(0x2C);
	while(j < 16)
	{
		writeBigRow(BigFont[offset][j]);
		j++;
		
	}
	
	
}
