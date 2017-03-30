/* AVR PRINTING (C) 2011 <>< Charles Lohr - This file may be licensed under the MIT/x11 or New BSD Licenses */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>


#if	defined (__AVR_ATtiny261__) || defined (__AVR_ATtiny261A__)
#define SPI_DDR_SET {DDRA&=0xFA;DDRA|=0x02;}
#define SPI_TINY

#elif	defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny44A__) || \
	defined (__AVR_ATtiny24__) || defined (__AVR_ATtiny24A__) || \
	defined (__AVR_ATtiny84__) || defined (__AVR_ATtiny84A__)  
#define SPI_DDR_SET {DDRA&=0xAF;DDRA|=0x20;}
#define SPI_TINY

#elif   defined (__AVR_ATmega168__)
#define SPI_DDR_SET {DDRB&=0xD3;DDRB|=0x10;} 
#define SPI_MEGA

#else
#error UNDEFINED PIN ASSIGNMENTS

#endif

//OPTIONAL!!!
#ifdef SPI_TINY
#define USE_SPI_INTERRUPT
#endif


volatile unsigned char ThisCharToSend = 0;
volatile unsigned char BackChar = 0;
#ifdef SPI_MEGA
ISR( SPI_STC_vect )
{
	BackChar = SPDR;
	SPDR = ThisCharToSend;
	ThisCharToSend = 0;
}
#elif defined( SPI_TINY )
ISR( USI_OVF_vect )
{
	USIBR = ThisCharToSend;
	ThisCharToSend = 0;
	USISR |= (1<<USIOIF); 
}
#else
#error NO_SPI_VECT_DEFINED
#endif

void sendchr( char c )
{
#ifdef USE_SPI_INTERRUPT
	ThisCharToSend = c;
	while( ThisCharToSend );
#else
	while(!(SPSR & (1<<SPIF)));
	SPDR = c;
#endif
}

#define sendstr( s ) { unsigned char rcnt; \
	for( rcnt = 0; s[rcnt] != 0; rcnt++ ) \
		sendchr( s[rcnt] ); }

void sendhex1( unsigned char i )
{
	sendchr( (i<10)?(i+'0'):(i+'A'-10) );
}
void sendhex2( unsigned char i )
{
	sendhex1( i>>4 );
	sendhex1( i&0x0f );
}
void sendhex4( unsigned int i )
{
	sendhex2( i>>8 );
	sendhex2( i&0xFF);
}


static int SPIPutCharInternal(char c, FILE *stream)
{
	sendchr( c );
}

static FILE mystdout = FDEV_SETUP_STREAM( SPIPutCharInternal, NULL, _FDEV_SETUP_WRITE );


void setup_spi( void )
{
	SPI_DDR_SET;

#ifdef SPI_TINY
	USICR = (1<<USIWM0) | (1<<USICS1) | (1<<USIOIE);
#elif defined( SPI_MEGA )

	SPCR = _BV(SPE) | _BV(SPIE);

#else
#error NO_SPI_VECT_DEFINED
#endif
	stdout = &mystdout;
}


