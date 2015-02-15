/*
    Copyright (C) 2015 <>< Charles Lohr


    Permission is hereby granted, free of charge, to any person obtaining a
	copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "ntscfont.h"

void delay_ms(uint32_t time) {
  uint32_t i;
  for (i = 0; i < time; i++) {
    _delay_ms(1);
  }
}

#define NOOP asm volatile("nop" ::)

void NumToText( char * c, uint8_t a )
{
	c[0] = (a/100)+'0';
	c[1] = ((a/10)%10)+'0';
	c[2] = (a%10)+'0';
	c[3] = 0;
}
void NumToText4( char * c, uint16_t a )
{
	c[0] = (a/1000)+'0';
	c[1] = ((a/100)%10)+'0';
	c[2] = ((a/10)%10)+'0';
	c[3] = (a%10)+'0';
	c[4] = 0;
}


EMPTY_INTERRUPT(TIM0_OVF_vect );


int main( )
{
	cli();

	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00;	/*No scalar*/

	PLLCSR = _BV(PLLE) | _BV( PCKE );

	DDRB = _BV(1);

	DDRB |= _BV(3);
	DDRB |= _BV(4);
	PORTB |= _BV(1);

	TCCR1 = _BV(CS10);// | _BV(CTC1); //Clear on trigger.
	GTCCR |= _BV(PWM1B) |  _BV(COM1B0);// | _BV(COM1B1);
	OCR1B = 2;
	OCR1C = 3;
	DTPS1 = 0;
	DT1B = _BV(0) | _BV(4);

	TCCR0A = 0;
	TCCR0B = _BV(CS01);
	TIMSK |= _BV(TOIE0);

	//CH3 on my AVR is OSCCAL=237 to 241
	OSCCAL = 239;
	#define LINETIME  11 //Linetime of 7..20 is barely valid. 

	#define NTSC_VH  {	DDRB=0; }
	#define NTSC_HI   { DDRB=_BV(3); }
	#define NTSC_LOW   { DDRB=_BV(4)|_BV(3); }

	uint8_t line, i;

	#define TIMEOFFSET .12
	#define CLKOFS .12

	uint8_t frame = 0, k, ctll;
	char stdsr[8*13];
	sprintf( stdsr, "Fr: " );
	sprintf( stdsr+8, "Hello<><" );
	sprintf( stdsr+16, "World<><" );
	sprintf( stdsr+24, "        " );
	sprintf( stdsr+32, "        " );
	sprintf( stdsr+40, "        " );
	sprintf( stdsr+48, "        " );
	sprintf( stdsr+56, "        " );
	sprintf( stdsr+64, "        " );
	sprintf( stdsr+72, "        " );
	sprintf( stdsr+80, "        " );
	sprintf( stdsr+88, "        " );


	ADMUX =/* _BV(REFS1)  |  _BV(ADLAR) | */ 1; //1 = PB2
	ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADPS2) | _BV(ADPS1);

	#define RESETCNT {TCNT0 = LINETIME; TIFR|=_BV(TOV0); GTCCR|=PSR0;sei();}
	#define WAITTCNT sleep_cpu();


 	sleep_enable();
	sei();

	uint8_t hh = 0;
#define HHS 128
	uint8_t history[HHS];

	uint16_t ovax = 0; //0..1024 = 0...5v
	uint8_t  ovax8 = 0;
	while(1)
	{

		frame++;
		//H = 1./15734.264 = 63.555 / 2 = 31.7775
		for( line = 0; line < 6; line++ )
		{ NTSC_LOW; _delay_us(2.3-TIMEOFFSET); NTSC_HI; _delay_us(29.5-TIMEOFFSET-CLKOFS); }
		for( line = 0; line < 6; line++ )
		{ NTSC_LOW; _delay_us(27.1-TIMEOFFSET); NTSC_HI; _delay_us(4.7-TIMEOFFSET-CLKOFS); }
		for( line = 0; line < 6; line++ )
		{ NTSC_LOW; _delay_us(2.3-TIMEOFFSET); NTSC_HI; _delay_us(29.5-TIMEOFFSET-CLKOFS); }

		RESETCNT;
		for( line = 0; line < 39; line++ )
		{
			NTSC_LOW;
			_delay_us(4.7-TIMEOFFSET);
			NTSC_HI;

			//Do whatever you want.
			switch (line)
			{
			case 0:
				NumToText( stdsr+4, frame );
				break;
			case 1:
				ovax = ADC;
				ovax8 = ovax >> 2;
				history[hh++] = ovax8;
				hh&=sizeof(history)-1;
				ovax = ovax * 49 + (ovax>>1);
				ovax/=10;
				break;
			case 2:
				NumToText( stdsr+24, ovax/1000 );
				stdsr[27] = '.';
				break;
			case 3:
				break;
			case 5:
				NumToText4( stdsr+27, ovax );
				stdsr[27] = '.';
				break;
			}

			WAITTCNT;
			RESETCNT;
		}

		for( line = 0; line < 2; line++ )
		{
			NTSC_LOW;
			_delay_us(4.7-TIMEOFFSET);
			NTSC_HI;
			WAITTCNT;
			RESETCNT;
		}

		for( line = 0; line < 128; line++ )
		{
			NTSC_LOW; _delay_us(4.7-TIMEOFFSET); 
			NTSC_HI; _delay_us(8-TIMEOFFSET-CLKOFS);

//#define LINETEST
#ifdef LINETEST
			NTSC_VH; _delay_us(8-TIMEOFFSET-CLKOFS);
			NTSC_HI; _delay_us(44.5);
#else
			ctll = line>>2;
			for( k = 0; k < 8; k++ )
			{
			uint8_t ch = pgm_read_byte( &font_8x8_data[(stdsr[k+((ctll>>3)<<3)]<<3)] + (ctll&0x07) );
			for( i = 0; i < 8; i++ )
			{
				if( (ch&1) )
				{
					NTSC_VH;
				}
				else
				{
					NTSC_HI;
					NOOP;
				}
				ch>>=1;
				NOOP; NOOP; NOOP; NOOP;
			}
					NTSC_HI;

			}

			NTSC_HI;// _delay_us(46-TIMEOFFSET-CLKOFS);
			WAITTCNT;
			RESETCNT;
#endif
		}
		for( line = 0; line < 92; line++ )
		{
			NTSC_LOW;
			_delay_us(4.7-TIMEOFFSET);
			NTSC_HI;
			_delay_us(10);
			uint8_t v = history[(hh-line+HHS-1)&(HHS-1)];
			while(v--) NOOP;
			NTSC_VH;
			_delay_us(1);
			NTSC_HI;
			WAITTCNT;
			RESETCNT;
		}
	}
	
	return 0;
} 
