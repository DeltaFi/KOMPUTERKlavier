/*
* KlavierKomputer Firmware
*
* Date: 25/2/2016
* Author: Dragos Rotaru
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/***** IO *****/
#define MIDI_IN PD0
#define MIDI_IN_PORT PORTD
#define MIDI_IN_DDR DDRD

#define SER PD6		//HC595
#define SER_PORT PORTD
#define SER_DDR DDRD

#define RCLK PB0	//HC595
#define RCLK_PORT PORTB
#define RCLK_DDR DDRB

#define SCLK PB1	//HC595
#define SCLK_PORT PORTB
#define SCLK_DDR DDRB

#define HDD1 PB2
#define HDD1_PORT PORTB
#define HDD1_DDR DDRB

#define HDD2 PB3
#define HDD2_PORT PORTB
#define HDD2_DDR DDRB

#define HDD3 PB4
#define HDD3_PORT PORTB
#define HDD3_DDR DDRB

#define BUZZ PD5
#define BUZZ_PORT PORTD
#define BUZZ_DDR DDRD

#define KBD PB6		//keyboard data
#define KBD_PORT PORTB
#define KBD_DDR DDRB
#define KBD_PIN PINB

#define KBC PB7		//keyboard clock
#define KBC_PORT PORTB
#define KBC_DDR DDRB
#define KBC_PIN PINB

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
*/


//Google PS/2 Scan Codes
unsigned char keynotes[] PROGMEM = {
//       0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
	 0, 0xD8,    0, 0xD4, 0xD2, 0xD0, 0xD1, 0xDB,    0, 0xD9, 0xD7, 0xD5, 0xD3, TO 1,    0,    0,    //0
	 0, 0x81,BO  1,    0, 0x80, TO 2,    0,    0,    0,    0,BO  2,BO  3,    0, TO 4, TO 3,    0,    //1
	 0,BO  6,BO  4,BO  5, TO 6,    0, TO 5,    0,    0,    0,BO  7,BO  0, TO 9, TO 7, TO 8,    0,    //2
	 0,BO 11,BO  9,BO 10,BO  8,TO 11,TO 10,    0,    0,    0,BO 13,BO 12,TO 13,TO 12,    0,    0,    //3
	 0,BO 14,    0,TO 14,TO 16,TO 17,TO 15,    0,    0,BO 16,BO 18,BO 15,BO 17,TO 18,    0,    0,    //4
	 0,    0,    0,    0,TO 19,TO 20,    0,    0,    0,BO 19,    0,TO 21,    0,TO 23,    0,    0,    //5
	 0,    0,    0,    0,    0,    0,TO 22,    0,    0, 0xC7,    0, 0xCA, 0xCD,    0,    0,    0,    //6
      0xC6,    0, 0xC8, 0xCB, 0xCC, 0xCE,    0,    0, 0xDA,    0, 0xC9,    0,    0, 0xCF,    0,    0,    //7
	 0,    0,    0, 0xD6,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    //0
	 0, 0x82,    0,    0, 0x83,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    //1
	 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    //2
	 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    //3
	 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    //4
	 0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    //5
	 0,    0,    0,    0,    0,    0,    0,    0,    0, 0xC3,    0,    0,    0xC2, 0,    0,    0,    //6
	 0xC0, 0xC1, 0,    0,    0,    0,    0,    0,    0,    0,    0xC5, 0,    0,    0xC4, 0,    0,    //7
};


ISR(USI_START_vect){
	USISR |= (0 << USICNT0) | (0 << USICNT1) | (0 << USICNT2) | (0 << USICNT3);
}

ISR(USI_OVERFLOW_vect){
	static unsigned char br = 0;  //bitwise: [shift][shift-l][shift-r][extend][break]
	unsigned char u = USIDR;

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

void OnProc( unsigned char key, char up ){

	unsigned char i = 0, j = 0;
	unsigned char keynote = pgm_read_word( &keynotes[key] );

	if( ( keynote & 0xC0 ) == 0xC0 ){

		if( up )
			return;

		keynote &= ~0xC0;

		//getting a command key.
		switch( keynote ){

		case 0: //Insert (upshift)
			if( shift < 53 )
				shift++;
			break;
		case 1: //Delete (downshift)
			if( shift > 0 )
				shift--;
			break;
		case 2: //Home (reset)
			shift = 12;
			break;
		case 3: //End (???)
			break;
		case 4: //Pg Up
			shift += 12;
			if( shift > 53 )
				shift = 53;
			break;
		case 5:
			shift -= 12;
			if( shift < 0 )
				shift = 0;
			break;

		case 6: //KP0
			Instrument = Instrument0;
			break;
		case 7: //KP1
			Instrument = Instrument1;
			break;
		case 8: //KP2
			Instrument = Instrument2;
			break;
		case 9: //KP3
			Instrument = Instrument3;
			break;
		case 10: //KP4
			Instrument = Instrument4;
			break;
		case 16: //F1
			PlayMode = PlayStandard;
			break;
		case 17: //F2
			PlayMode = PlayArp;
			break;
		case 18: //F3
			PlayMode = PlayArp2;
			break;
		case 19: //F4
			PlayMode = PlaySlide;
			break;

		case 24: progressspeed ^= 1; break;
		case 25: progressspeed ^= 2; break;
		case 26: progressspeed ^= 4; break;
		case 27: progressspeed ^= 8; break;
		}

		return;
	}

	if( up ){

		for( ; i < MAXKEYSDOWN; ++i ){

			if( keysdown[i] == key ){

				unsigned char toremove;
				unsigned char note = pgm_read_byte( &keynotes[key] );
				keysdown[i] = 0;

				if( note && (!(note & 0x80) ) ){

					for( j = 0; j < keyordertail; j++ )
						if( keyorder[j] == i )
							toremove = j;
					for( j = toremove; j < keyordertail; j++ ){

						keyorder[j] = keyorder[j+1];
					}

					keyordertail--;
					if( keyordertail == 0 ){

						keyorderslur = 0;
					}
				}
			}
		}
	}
	else{
		//Aleady Depressed.
		for( ; i < MAXKEYSDOWN; ++i ){

			if( keysdown[i] == key ){
				return;
			}
		}

		//If not, make it pressed.
		for( i = 0; i < MAXKEYSDOWN; ++i ){

			if( keysdown[i] == 0  ){

				break;
			}
		}

		if( i != MAXKEYSDOWN ){

			keysdown[i] = key;
			timesincehit[i] = 0;
			voice[i] = 0;
			unsigned char note = pgm_read_byte( &keynotes[key] );

			if( note && (!(note&0x80)) ){

				if( keyordertail )
					keyorderslur = 1;
				keyorder[keyordertail] = i;
				keyordertail++;
			}
		}
	}
}

void setupKeyboard(){
	KBD_DDR |= (0 << KBD);
	KBD_PORT |= (0 << KBD);
	KBC_DDR |= (0 << KBC);
	KBC_PORT |= (0 << KBC);
	USICR |= (1 << USISIE) | (1 << USIOIE) | (1 << USIWM1 ) | (0 << USIWM0) | (1 << USICS1) | (1 << USICS0) | (0 <<  USICLK) | (0 <<  USITC);
}

void setupMidi(){
}

void setupShiftReg(){
}

void setupPWM(){
}

int main(void) {
	setupKeyboard();
	setupMidi();
	setupShiftReg();
	setupPWM();

	while(1){
		_delay_ms(100);
	}

	return 0;
}
