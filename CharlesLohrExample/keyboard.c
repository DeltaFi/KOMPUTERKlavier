//AVR PS/2 Keyboard Control, Copyright 2011 <>< Charles Lohr, under the MIT/x11 license.

//As of now, on the ATMega, with Os, it takes about 480 bytes of flash.

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "keyboard.h"
//#include "avr_print.h"
#include <stdio.h>

/*
unsigned char keychars[] PROGMEM = {
//       0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
	 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , '\t', '`' , 0,    //0
	 0   , 0   , 0   , 0   , 0   , 'q' , '1' , 0   , 0   , 0   , 'z' , 's' , 'a' , 'w' , '2' , 0,    //1
	 0   , 'c' , 'x' , 'd' , 'e' , '4' , '3' , 0   , 0   , ' ' , 'v' , 'f' , 't' , 'r' , '5' , 0,    //2
	 0   , 'n' , 'b' , 'h' , 'g' , 'y' , '6' , 0   , 0   , 0   , 'm' , 'j' , 'u' , '7' , '8' , 0,    //3
	 0   , ',' , 'k' , 'i' , 'o' , '0' , '9' , 0   , 0   , '.' , '/' , 'l' , ';' , 'p' , '-' , 0,    //4
	 0   , 0   , '\'', 0   , '[' , '=' , 0   , 0   , 0   , 0   , '\n', ']' , 0   , '\\', 0   , 0,    //5 //NOTE: LAST CAPS/SHIFT ROW
	 0   , 0   , 0   , 0   , 0   , 0   , 0x08, 0   , 0   , '1' , 0   , '4' , '7' , 0   , 0   , 0,    //6
	 '0' , '.' , '2' , '5' , '6' , '8' , 0x1B, 0   , 0   , '+' , '3' , '-' , '*' , '9' , 0   , 0,    //7
};

unsigned char keyshifts[] PROGMEM = {
//       0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
	 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , 0   , '\t', '~' , 0,    //0
	 0   , 0   , 0   , 0   , 0   , 'Q' , '!' , 0   , 0   , 0   , 'Z' , 'S' , 'A' , 'W' , '@' , 0,    //1
	 0   , 'C' , 'X' , 'D' , 'E' , '$' , '#' , 0   , 0   , ' ' , 'V' , 'F' , 'T' , 'R' , '%' , 0,    //2
	 0   , 'N' , 'B' , 'H' , 'G' , 'Y' , '^' , 0   , 0   , 0   , 'M' , 'J' , 'U' , '&' , '*' , 0,    //3
	 0   , '<' , 'K' , 'I' , 'O' , ')' , '(' , 0   , 0   , '>' , '?' , 'L' , ':' , 'P' , '_' , 0,    //4
	 0   , 0   , '"' , 0   , '{' , '+' , 0   , 0   , 0   , 0   , '\n', '}' , 0   , '?' , 0   , 0,    //5
};
*/
ISR( USART_RX_vect )
{
	static unsigned char br = 0;  //bitwise: [shift][shift-l][shift-r][extend][break]

	unsigned char u = UDR0;
//	sei();

	PORTB ^= 2;

	if( u == 0xF0 )
	{
		br |= 1;
	}
	else if( u == 0xE0 )
	{
		br |= 2;
	}
	else if( u < 0x84 )
	{
		if( br & 2 ) u |= 0x80;
		OnProc( u, (br & 1) );
		br &= 0xFC;
	}
	else
	{
		br &= 0xFC;
	}
}


void SetupKeyboard()
{
	DDRD &= 0xEC; //PD4 in and PD0 in
	PORTD &= _BV(4);  //Clock should be default-low

//	DDRD = 0x00;
//	PORTD = 0x00;

//	UMSEL = 1
//	UCPOL = 0
//	1 Stop bit
//	1 parity  (UPMn1 = 1) ODD

	UCSR0B = _BV(RXEN0) | _BV(RXCIE0);
	UCSR0C = _BV(UMSEL00) 			//Synchronous
		| _BV(UPM00) | _BV(UPM01)	//Parity
		| _BV(UCSZ01) | _BV(UCSZ00); //8-bit words


}
