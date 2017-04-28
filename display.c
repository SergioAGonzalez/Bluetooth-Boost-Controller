#include "font8x8.h"
#include "LCD.c"
//Cons tant s f o r lookup f ont
unsigned const char zer = 48 ;

void writeLetter(int offset, unsigned short x, unsigned short y)
{
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned char letterColor = 0;
	setArea(x,x+15,y,y+15);
	writeCmd(0x2C);
	for(i = 0;i < 32;i++)
	{
		letterColor = font8x8[offset+i];
		for(j = 0; j< 9; j++){
			if((letterColor & 0x80) == 0)
			{
				writeDat2(black);
			}
			else{
				writeDat2(white);
			}
			letterColor = letterColor <<1;
		}
	}
	
	
}
