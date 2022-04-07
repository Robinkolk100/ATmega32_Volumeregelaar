/*
 * Volumeregelaar.c
 *
 * Created: 06/04/2022 16:32:02
 * Author : Robin Kolk
 */ 

//Define includes
#define AVR_ATmega32
#define  _OPEN_SYS_ITOA_EXT
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <avr/io.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#include "avr/iom32.h"
//F_CPU
#define F_CPU 1000000UL // oscillator frequency

//Define lcd
#define lcdcommand(d) (lcdwrite((d),PORTD&0xDF));  // RS low
#define lcdputc(d) (lcdwrite((d),PORTD|0x20));    // RS high

//Global var
int adcResultH[4]; // 4 digit number
int adcResultL[4]; // 4 digit number

//Set lcd to write data
void lcdwrite(char d, char type)
{
	PORTD = type;			// RS low or high
	PORTD = PORTD|0x80;		// make E high
	PORTC = d;				// assign data
	PORTD = PORTD & 0x7F;	// make E low
	_delay_us(1520);		// wait 1,52 ms worst case delay
}


// initialiseert het LCD display
void initlcd(void)
{
	DDRC = 0xFF;		// port D: 8-bit data
	DDRD = 0xFF;		// pin 0 and 1 port B: RS en E
	_delay_ms(15);		// delay of 15 ms
	lcdcommand(0x38);   // 8-bits, 2 lines, 5x8 font
	lcdcommand(0x38);   // 8-bits, 2 lines, 5x8 font
	lcdcommand(0x38);   // 8-bits, 2 lines, 5x8 font
	lcdcommand(0x38);   // 8-bits, 2 lines, 5x8 font
	lcdcommand(0x0C);   // display on, cursor off, blink off
	lcdcommand(0x06);   // move cursor right
	lcdcommand(0x01);   // clear display
}


// Zorgt ervoor dat de cursor verplaatst kan worden tussen de regels 1 t/m 4.
void moveCursor(int row, int column){
	if(row == 1){
		lcdcommand(0x80); // Begin 1ste rij
		}else if(row == 2){
		lcdcommand(0xC0); // Begin 2de rij
		}else if(row == 3){
		lcdcommand(0x90); // Begin 3de rij
		}else if(row == 4){
		lcdcommand(0xD0); // Begin 4de rij
	}
	
	int i = 1;
	while(i < column){
		lcdcommand(0x14);
		i++;
	}
}
void putString(char * string){  //(text, row, column)
	
	int j = 0;

	
	while(string[j] != '\0'){

		
		lcdputc(string[j]);
		j++;
	}
}

int main(void)
{
	initlcd();
	
	//Configure the ADC
	ADCSRA |= 1<<ADPS2; // ADC prescaler 16 division factor
	//ADMUX |= 1<<ADLAR; // ADC Data on 8 Bit nummer
	//ADMUX |= 0<<ADLAR; //
	ADMUX |= 1<<REFS0 | 1<<REFS1 ; // AVCC external capacitoy at AREF pin 0b1100 0000 | 0b1100 0001
	ADCSRA |= 1<<ADIE; // ADC Interrupt enable 
	ADCSRA |= 1<<ADEN; // Turn on ADC control and status register A 

	sei(); //Global Enable interrupts

	ADCSRA |= 1<<ADSC; // // ADC Start first conversion	
	while (1)
	{
	}
	
	return (0);
}

ISR(ADC_vect) // interrupt service routine 
{
	uint8_t theLow = ADCL; //de low adc conversion 
	uint16_t theHigh = ADCH<<8 | theLow ; // shitf de high register 8 plaatsen over om plaats te maken voor de low  is 10 bit result
	
	switch(ADMUX)
	{
		case 0xC0:
			moveCursor(2,1);
			itoa(theHigh, adcResultH, 10); // Base10 nummer leesbaar voor mensen maakt eem STRING
			
			putString(adcResultH);
			putString(" "); // exstra nul weg halen
			ADMUX = 0xC1;
			break;
			
		case 0xC1:
			moveCursor(4,1);
			
			itoa(theHigh, adcResultL, 10); // Base10 nummer leesbaar voor mensen maakt eem STRING
			
			putString(adcResultL);
			putString(" "); // exstra nul weg halen
			ADMUX = 0xC0;
			break;
			
		default:
			break;			
	}
		
	moveCursor(1,1);  // Print waarde op LCD scherm eerste rij, eerste colum
	putString("L:");
	
	moveCursor(3,1);  // Print waarde op LCD scherm eerste rij, eerste colum
	putString("R:"); 
	
	ADCSRA |= 1<<ADSC; // Start next conversion
}

