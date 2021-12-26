/* time4io.c
   Written by Ludvig Edvinson, 2021

   For copyright and licensing, see file COPYING */
#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"

int getsw( void ){
	int switchstatus = 0xf&(PORTD>>8);
	return switchstatus;
}

int getbtns( void ){
	int buttonstatus = 0x7&(PORTD>>5);
	buttonstatus += ((0x1&(PORTF >> 1)) * 8);
	return buttonstatus;
}
