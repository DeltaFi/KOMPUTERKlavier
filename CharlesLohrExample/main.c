 /* General demo program, Copyleft 2008 Charles Lohr */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "keyboard.h"
#include "frequencies.h"

unsigned short xorfn1, xorfn2, xorfn3, xorfn4;

unsigned char noisemask = 0;


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


#define TO 0+
#define BO 12+

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

#define MAXKEYSDOWN 5
unsigned char keysdown[MAXKEYSDOWN] = {};
unsigned short voice[MAXKEYSDOWN];
unsigned short timesincehit[MAXKEYSDOWN];
unsigned short melodymodetime;
unsigned short melodyvoice;
unsigned char keyorder[MAXKEYSDOWN];
unsigned char keyordertail = 0;
unsigned char keyorderslur = 0;

unsigned char progressspeed = 1;
char shift = 12;

/******************************* INSTRUMENTS ********************************/


unsigned char Instrument0( unsigned char voiceo, unsigned char tremolo )
{
	if( voiceo & 0x80 )
		return 0x7F - (voiceo & 0x7F);
	else
		return voiceo;
}

unsigned char Instrument1( unsigned char voiceo, unsigned char tremolo )
{
	return voiceo;
}

unsigned char Instrument2( unsigned char voiceo, unsigned char tremolo )
{
	if( voiceo & 0x80 )
		return 0xFF;
	else
		return 0;
}

unsigned char Instrument3( unsigned char voiceo, unsigned char tremolo )
{
	if( tremolo & 0x80 ) tremolo = 0x7F - (tremolo&0x7F);

	if( voiceo > tremolo )
		return 0xFF;
	else
		return 0;
}

unsigned char Instrument4( unsigned char voiceo, unsigned char tremolo )
{
//	return voiceo | (voiceo<<2) | (voiceo<<3) | (voiceo<<1);
	return (voiceo) | ((voiceo>>1) + voiceo);
}

unsigned char (*Instrument)( unsigned char voiceo, unsigned char tremolo ) = Instrument0;

unsigned short GetNoise()
{
	unsigned char i;
	unsigned short sout = 0;
	for( i = 0; i < MAXKEYSDOWN; ++i )
	{
		unsigned char k = keysdown[i];
		unsigned char cmd;
		if( !k )
			continue;
		cmd = pgm_read_byte( &keynotes[k] );
		if( cmd & 0x80 )
		{
			if( cmd == 0x80 )	sout += (xorfn1 & 0xff);
			if( cmd == 0x81 )	sout += (xorfn2 & 0xff);
			if( cmd == 0x82 )	sout += (xorfn3 & 0xff);
			if( cmd == 0x83 )	sout += (xorfn4 & 0xff);
		}
	}
	return sout;
}

void PlayStandard()
{
	unsigned char i;
	unsigned short sout = 0;
	for( i = 0; i < MAXKEYSDOWN; ++i )
	{
		unsigned char k = keysdown[i];
		unsigned char cmd;
		if( !k )
			continue;
		cmd = pgm_read_byte( &keynotes[k] );
		if( cmd & 0x80 )
		{
			if( cmd == 0x80 )	sout += (xorfn1 & 0xff);
			if( cmd == 0x81 )	sout += (xorfn2 & 0xff);
			if( cmd == 0x82 )	sout += (xorfn3 & 0xff);
			if( cmd == 0x83 )	sout += (xorfn4 & 0xff);
		}
		else if( cmd )
		{
			unsigned short vc = voice[i];
			vc += pgm_read_word( &notes[cmd-1+shift] );
			voice[i] = vc;

			unsigned char voiceo = vc>>8;
			unsigned short tsh = (timesincehit[i]+=progressspeed);

			unsigned char tremolo = 0;

			tremolo = tsh >> 8;

			sout += Instrument( voiceo, tremolo );
		}
	}


	OCR0A = OCR0B = sout>>2;
}


unsigned char GetSingleKey()
{
	unsigned char cmd;
	unsigned char k;

	if( keyordertail == 0 )
	{
		return 0;
	}

	k = keysdown[keyorder[keyordertail-1]];

	cmd = pgm_read_byte( &keynotes[k] );

	return cmd|(keyorderslur?0x80:0x00);
}



void PlayArp()
{
	static unsigned char cnote;
	unsigned char note = GetSingleKey();
	unsigned short sout = 0;
	static unsigned char cmode;

	if( note )
	{
		unsigned short vc = melodyvoice;
		unsigned short vtt = pgm_read_word( &notes[(cnote&0x7f)-1+shift] );
		unsigned char voiceo = vc>>8;


		melodymodetime = melodymodetime + progressspeed;

		vc += vtt;

		if( melodymodetime & 0x4000 )
		{
			vc += vtt>>1;
			cmode = 0;
		}
		else
		{
			if( cmode == 0 )
			{
				cnote = note;
			}
			cmode = 1;

		}
		melodyvoice = vc;

		unsigned char tremolo = 0;

		tremolo = melodymodetime >> 8;

		sout += Instrument( voiceo, tremolo );
	}
	else
	{
		cmode = 0;
		melodymodetime = 0;
		sout = 127;
	}

	sout += GetNoise();

	OCR0A = OCR0B = sout>>2;
}

void PlayArp2()
{
	unsigned char note = GetSingleKey();
	unsigned short sout = 0;
	static unsigned char cnote;
	static unsigned char cmode;

	if( note )
	{
		unsigned short vc = melodyvoice;
		unsigned short vtt = pgm_read_word( &notes[(cnote&0x7f)-1+shift] );

		unsigned char voiceo = vc>>8;
		unsigned short tsh = (melodymodetime+=progressspeed);


		vc += vtt;

		if( melodymodetime & 0x4000 )
		{
			if( melodymodetime & 0x8000 )
			{
				vc += vtt>>1;
			}
			else
			{
				vc += vtt>>2;
			}

			cmode = 0;
		}
		else
		{
			if( cmode == 0 )
			{
				cnote = note;
			}
			cmode = 1;
		}
		melodyvoice = vc;

		unsigned char tremolo = 0;

		tremolo = tsh >> 8;

		sout += Instrument( voiceo, tremolo );
	}
	else
	{
		melodymodetime = 0;
		sout = 127;
		cmode = 0;
	}

	sout += GetNoise();

	OCR0A = OCR0B = sout>>2;
}

void PlaySlide()
{
	unsigned char note = GetSingleKey();
	unsigned short sout = 0;
	unsigned char i;

	static unsigned short thisspeed;
	unsigned short sttarget = pgm_read_word( &notes[(note&0x7f)-1+shift] );

	static unsigned short melodyspace = 0;

	if( note & 0x80 )
	{
		if( (melodyspace+=progressspeed * thisspeed)>0x8000)
		{
			melodyspace = 0;
			if( thisspeed < sttarget )
				thisspeed++;
			else if( thisspeed > sttarget )
				thisspeed--;
		}

	}
	else
	{
		thisspeed = sttarget;
	}

	if( note )
	{
		unsigned short vc = melodyvoice;

		unsigned char voiceo = vc>>8;
		unsigned short tsh = (melodymodetime+=progressspeed);


		vc += thisspeed;

		melodyvoice = vc;

		unsigned char tremolo = 0;

		tremolo = tsh >> 8;

		sout += Instrument( voiceo, tremolo );
	}
	else
	{
		melodymodetime = 0;
		sout = 127;
	}

	for( i = 0; i < MAXKEYSDOWN; ++i )
	{
		unsigned char k = keysdown[i];
		unsigned char cmd;
		if( !k )
			continue;
		cmd = pgm_read_byte( &keynotes[k] );
		if( cmd & 0x80 )
		{
			if( cmd == 0x80 )	sout += (xorfn1 & 0xff);
			if( cmd == 0x81 )	sout += (xorfn2 & 0xff);
			if( cmd == 0x82 )	sout += (xorfn3 & 0xff);
			if( cmd == 0x83 )	sout += (xorfn4 & 0xff);
		}
	}

	OCR0A = OCR0B = sout>>2;
}

void (*PlayMode)() = PlayStandard;

void OnProc( unsigned char key, char up )
{
	unsigned char i = 0, j = 0;

	unsigned char keynote = pgm_read_word( &keynotes[key] );

	if( ( keynote & 0xC0 ) == 0xC0 )
	{
		if( up )
			return;

		keynote &= ~0xC0;

		//getting a command key.
		switch( keynote )
		{
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

	if( up )
	{
		for( ; i < MAXKEYSDOWN; ++i )
		{
			if( keysdown[i] == key )
			{
				unsigned char toremove;

				unsigned char note = pgm_read_byte( &keynotes[key] );
				keysdown[i] = 0;


				if( note && (!(note & 0x80) ) )
				{
					for( j = 0; j < keyordertail; j++ )
						if( keyorder[j] == i )
							toremove = j;
					for( j = toremove; j < keyordertail; j++ )
					{
						keyorder[j] = keyorder[j+1];
					}

					keyordertail--;
					if( keyordertail == 0 )
					{
						keyorderslur = 0;
					}
				}
			}
		}
	}
	else
	{
		//Aleady Depressed.
		for( ; i < MAXKEYSDOWN; ++i )
		{
			if( keysdown[i] == key )
			{
				return;
			}
		}

		//If not, make it pressed.
		for( i = 0; i < MAXKEYSDOWN; ++i )
		{
			if( keysdown[i] == 0  )
			{
				break;
			}
		}

		if( i != MAXKEYSDOWN )
		{
			keysdown[i] = key;
			timesincehit[i] = 0;
			voice[i] = 0;

			unsigned char note = pgm_read_byte( &keynotes[key] );

			if( note && (!(note&0x80)) )
			{
				if( keyordertail )
					keyorderslur = 1;
				keyorder[keyordertail] = i;
				keyordertail++;
			}
		}
	}
}

unsigned char sframe = 0;

ISR(TIMER1_COMPA_vect)
{

	++sframe;

	if( sframe & 1 )	xorfn1 = (xorfn1 >> 1) ^ (-(xorfn1 & 1u) & 0xB400u);
	else if( sframe & 2 )	xorfn2 = (xorfn2 >> 1) ^ (-(xorfn2 & 1u) & 0xB400u);
	else if( sframe & 4 )	xorfn3 = (xorfn3 >> 1) ^ (-(xorfn3 & 1u) & 0xB400u);
	else if( sframe & 8 )	xorfn4 = (xorfn4 >> 1) ^ (-(xorfn4 & 1u) & 0xB400u);

	PlayMode();
}


void SetupSynth()
{
	DDRD |= 0x60;
	TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00);
	TCCR0B = _BV(CS00);
	TCNT0 = 0;


	TCNT1 = 0;
	OCR1A = CDIV;
	TCCR1A = 0;
	TCCR1B = _BV(CS10) | _BV(WGM12);
	TIMSK1 = _BV(OCIE1A); //Enable overflow mark

	xorfn1 = xorfn2 = xorfn3 = xorfn4 = 0xACE1;
}

void SetupClock()
{
	CLKPR = _BV( CLKPCE );
	CLKPR = 0; //No divide
}

int main()
{
	cli();

	DDRB = 0x03;

	SetupKeyboard();
	SetupSynth();
	SetupClock();
	sei();

	while(1)
	{
		_delay_ms(100);

	}
	return 0;
}
