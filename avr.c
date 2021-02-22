#ifndef F_CPU
#define F_CPU 16000000UL // 16 MHz clock speed
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>
#define delayTime 500
/*
(PCINT14/RESET)      	PC6 /|1  \_/ 28|\ PC5 (ADC5/SCL/PCINT13)
(PCINT16/RXD)        	PD0 /|2      27|\ PC4 (ADC4/SDA/PCINT12)
(PCINT17/TXD)        	PD1 /|3      26|\ PC3 (ADC3/PCINT11)
(PCINT18/INT0)       	PD2 /|4      25|\ PC2 (ADC2/PCINT10)
(PCINT19/OC2B/INT1)  	PD3 /|5      24|\ PC1 (ADC1/PCINT9)
(PCINT20/XCK/T0)     	PD4 /|6      23|\ PC0 (ADC0/PCINT8)
VCC                  	    /|7      22|\     GND
GND                  	    /|8      21|\     AREF
(PCINT6/XTAL1/TOSC1) 	PB6 /|9      20|\     AVCC
(PCINT7/XTAL2/TOSC2) 	PB7 /|10     19|\ PB5 (SCK/PCINT5)
(PCINT21/OC0B/T1)    	PD5 /|11     18|\ PB4 (MISO/PCINT4)
(PCINT22/OC0A/AIN0) 	PD6 /|12     17|\ PB3 (MOSI/OC2A/PCINT3)
(PCINT23/AIN1)      	PD7 /|13     16|\ PB2 (SS/OC1B/PCINT2)
(PCINT0/CLKO/ICP1)      PB0 /|14     15|\ PB1 (OC1A/PCINT1)
*/

#define enable            4
#define registerselection 5
#define input01 3
#define input02 2
#define input03 1
#define inputBuzz 0
#define waitTime 5
#define buzzerWaitTime 3

static volatile int sec = 0;
static volatile int min = 34;
static volatile int hor = 3;

static volatile int timerSec = 0;
static volatile int timerMin = 0;
static volatile int timerHor = 0;

void send_a_command(unsigned char command)
{
    PORTD = command;
    PORTC &= ~ (1<<registerselection) | (1 << input01) | (1 << input02) | (1<<input03);
    PORTC |= 1<<enable;
    _delay_ms(3);
    PORTC &= ~1<<enable | (1 << input01) | (1 << input02) | (1<<input03);
    PORTD = 0xFF;
}

void send_a_character(unsigned char character)
{
    PORTD = character;
    PORTC |= 1<<registerselection;
    PORTC |= 1<<enable;
    _delay_ms(3);
    PORTC &= ~1<<enable | (1 << input01) | (1 << input02) | (1<<input03);
    PORTD = 0xFF;
}

void send_a_string(char *string_of_characters)
{
    while(*string_of_characters > 0)
    {
        send_a_character(*string_of_characters++);
    }
}

int setup()
{
	DDRD = ~0x00; //set all D ports to outputs 
	DDRC |= (1 << enable) | (1 << registerselection) | (1 << inputBuzz);
	DDRC &= ~(1<<input01 | 1<<input02 | 1<<input03);
	PORTC |= (1 << input01) | (1 << input02) | (1<<input03);

	send_a_command(0x01); //Clear Screen 0x01 = 00000001
	_delay_ms(50);
	send_a_command(0x38);//telling lcd we are using 8bit command /data mode

	_delay_ms(50);

	send_a_command(0b00001111);//LCD SCREEN ON and courser blinking
	return 0;
}

int displayTime(char *hourDisplay, char *minDisplay, char *secDisplay, char *timeModeDisplay, int timeMode)
{
	char *charArray = "Time: ";
  		send_a_string(charArray);

  		send_a_command(0x80 + 6);

  		//hours
  		itoa(hor/10, hourDisplay, 10);
  		send_a_string(hourDisplay);
  		itoa(hor%10, hourDisplay, 10);
  		send_a_string(hourDisplay);
  		
  		send_a_command(0x80 + 8);

  		//minutes 
  		send_a_character(':');
  		itoa(min/10, minDisplay, 10);
  		send_a_string(minDisplay);
  		itoa(min%10, minDisplay, 10);
  		send_a_string(minDisplay);

  		send_a_command(0x80 + 11);

  		//seconds 
  		send_a_character(':');

  		send_a_command(0x80 + 12);
  		itoa(sec/10, secDisplay, 10);
  		send_a_string(secDisplay);
  		itoa(sec%10, secDisplay, 10);
  		send_a_string(secDisplay);

  		send_a_character(timeModeDisplay[timeMode]);

  		return 0;
}


int displayTimer(char *hourTimerDisplay, char *minTimerDisplay, char *secTimerDisplay)
{
	  send_a_command(0x80 + 0x40);
  			send_a_string("Timer:");

  			//hours
	  		itoa(timerHor/10, hourTimerDisplay, 10);
	  		send_a_string(hourTimerDisplay);
	  		itoa(timerHor%10, hourTimerDisplay, 10);
	  		send_a_string(hourTimerDisplay);
	  		
	  		send_a_command(0x80 + 0x40 + 8);

	  		//minutes 
	  		send_a_character(':');
	  		itoa(timerMin/10, minTimerDisplay, 10);
	  		send_a_string(minTimerDisplay);
	  		itoa(timerMin%10, minTimerDisplay, 10);
	  		send_a_string(minTimerDisplay);

	  		send_a_command(0x80 + 0x40 + 11);

	  		//seconds 
	  		send_a_character(':');

	  		send_a_command(0x80 + 0x40 + 12);
	  		itoa(timerSec/10, secTimerDisplay, 10);
	  		send_a_string(secTimerDisplay);
	  		itoa(timerSec%10, secTimerDisplay, 10);
	  		send_a_string(secTimerDisplay);

	  	
}

int changeTimer(int *timerStarted, int *timeMode, int *buzzarStart, int *buzzerCount)
{
	if(*timerStarted == 1)
	{
		if(timerHor == 00 && timerMin == 00 && timerSec == 00 )
		{
		  	timerHor = 0;
		  	timerMin = 0;
		  	timerSec = 0;
		  	*timerStarted = 0;
		  	timeMode = 0;
		  	*buzzarStart = 1;
			*buzzerCount = 0;
		}
		else
		{
			if(timerSec > 0)
				timerSec--;
			if(timerSec == 0 && timerMin > 0)
			{
			  	timerMin--;
			  	timerSec = 59;
			}
			if(timerMin==0 && timerHor > 0)
			{
				timerHor--;
				timerMin=59;
			}	
		}
	}
}

int buzzerStarting(int *buzzerCount, int *buzzerStart)
{
	if(*buzzerCount < buzzerWaitTime)
	{
		(*buzzerCount)++;
		PORTC |= (1 << inputBuzz);
	}
	else
	{
		*buzzerStart =0;
	}
}


int main(void)
{
	char hourDisplay[2];
	char minDisplay[2];
	char secDisplay[2];

	char timeModeDisplay[2] = {'D','T'};
	int timeMode = 0;
	int timerStarted = 0;

	int buzzarStart = 0;
	int buzzerCount = 0;

	char hourTimerDisplay[2];
	char minTimerDisplay[2];
	char secTimerDisplay[2];

	int constant = 59;
	unsigned long randomVar = 0;
	int buttonpressed = 0;
	int buttonpressed01 = 0;
	int buttonpressed02 = 0;
	int buttonpressed03 = 0;
	int index = 2;

	int relConfidenceLevel  =0;
	int pressConfidenceLevel=0;
	int relConfidenceLevel01  =0;
	int pressConfidenceLevel01 =0;
	int relConfidenceLevel02  =0;
	int pressConfidenceLevel02 =0;
	int relConfidenceLevel03  =0;
	int pressConfidenceLevel03 =0;

	setup();

  	while(1) //infinite loop
  	{
  		//switch for normal time editing and Timer mode
  		if(bit_is_clear(PINC, input03) && timeMode == 0)
	  	{	
	  		pressConfidenceLevel02++;
	  		relConfidenceLevel02 = 0;
	  		if(pressConfidenceLevel02 > waitTime)
	  		{
		  		if(!buttonpressed02)
		  		{
		  			if(min < 60)
		  			{
		  				timeMode ^= 1;
		  			}
		  			_delay_ms(3);
		  		}
		  		pressConfidenceLevel02 = 0;
		  	}
	  	}
	  	else
	  	{
	  		relConfidenceLevel02++;
	  		pressConfidenceLevel02 = 0;

	  		if(relConfidenceLevel02 > waitTime)
	  		{
	  			relConfidenceLevel02 = 0;
	  			buttonpressed = 0;
	  		}
	  	}

	  	//normal editing time mode
  		if(timeMode == 0)
  		{
	  		if(bit_is_clear(PINC, input01))
	  		{	
	  			pressConfidenceLevel01++;
	  			relConfidenceLevel01 = 0;
	  			if(pressConfidenceLevel01 > waitTime)
	  			{
		  			if(!buttonpressed)
		  			{
		  				if(min < 60)
		  					min++;
		  				_delay_ms(3);
		  			}
		  			pressConfidenceLevel01 = 0;
		  		}
	  		}
	  		else
	  		{
	  			relConfidenceLevel01++;
	  			pressConfidenceLevel01 = 0;

	  			if(relConfidenceLevel01 > waitTime)
	  			{
	  				relConfidenceLevel01 = 0;
	  				buttonpressed = 0;
	  			}
	  		}

	  		if(bit_is_clear(PINC, input02))
	  		{	
	  			pressConfidenceLevel++;
	  			relConfidenceLevel = 0;
	  			if(pressConfidenceLevel > waitTime)
	  			{
		  			if(!buttonpressed01)
		  			{
		  				if(hor < 24)
		  					hor++;
		  				_delay_ms(3);
		  			}
		  			pressConfidenceLevel = 0;
		  		}
	  		}
	  		else
	  		{
	  			relConfidenceLevel++;
	  			pressConfidenceLevel = 0;

	  			if(relConfidenceLevel > waitTime)
	  			{
	  				relConfidenceLevel = 0;
	  				buttonpressed01 = 0;
	  			}
	  		}
	  	}
	  	else //timer mode
	  	{
	  		if(bit_is_clear(PINC, input01))
	  		{	
	  			pressConfidenceLevel01++;
	  			relConfidenceLevel01 = 0;
	  			if(pressConfidenceLevel01 > waitTime)
	  			{
		  			if(!buttonpressed)
		  			{
		  				if(min < 60)
		  					timerMin++;
		  				_delay_ms(3);
		  			}
		  			pressConfidenceLevel01 = 0;
		  		}
	  		}
	  		else
	  		{
	  			relConfidenceLevel01++;
	  			pressConfidenceLevel01 = 0;

	  			if(relConfidenceLevel01 > waitTime)
	  			{
	  				relConfidenceLevel01 = 0;
	  				buttonpressed = 0;
	  			}
	  		}

	  		if(bit_is_clear(PINC, input02))
	  		{	
	  			pressConfidenceLevel++;
	  			relConfidenceLevel = 0;
	  			if(pressConfidenceLevel > waitTime)
	  			{
		  			if(!buttonpressed01)
		  			{
		  				if(timerHor < 24)
		  					timerHor++;
		  				_delay_ms(3);
		  			}
		  			pressConfidenceLevel = 0;
		  		}
	  		}
	  		else
	  		{
	  			relConfidenceLevel++;
	  			pressConfidenceLevel = 0;

	  			if(relConfidenceLevel > waitTime)
	  			{
	  				relConfidenceLevel = 0;
	  				buttonpressed01 = 0;
	  			}
	  		}

	  		// decrease time for timer if third button is clicked 
	  		// second click will exit timer mode
	  		if(bit_is_clear(PINC, input03) && timeMode == 1)
	  		{		
		  		pressConfidenceLevel03++;
		  		relConfidenceLevel03 = 0;
		  		if(pressConfidenceLevel03 > waitTime)
		  		{
			  		if(!buttonpressed03)
			  		{
			  			if(timerStarted == 1)
			  			{
			  				timerHor = 0;
				  			timerMin = 0;
				  			timerSec = 0;
				  			timerStarted = 0;
				  			timeMode = 0;
			  			}
			  			else
			  			{
			  				buzzerCount=0;
			  				timerStarted = 1;
			  				_delay_ms(3);
			  			}
			  		}
			  		pressConfidenceLevel03 = 0;
			  	}
		  	}
		  	else
		  	{
		  		relConfidenceLevel03++;
		  		pressConfidenceLevel03 = 0;

		  		if(relConfidenceLevel03 > waitTime)
		  		{
		  			relConfidenceLevel03 = 0;
		  			buttonpressed = 0;
		  		}
		  	}


	  	}

  		
	  	displayTime(hourDisplay, minDisplay, secDisplay, timeModeDisplay ,timeMode);

  		if(timeMode == 1)
  		{
  			displayTimer(hourTimerDisplay, minTimerDisplay, secTimerDisplay);
  			//Decrease time if timer is started change mode once timer is done
		  	//if sec is < then 60
		  	changeTimer(&timerStarted, &timeMode, &buzzarStart, &buzzerCount);
		  	

		}

  		if(buzzarStart)
		{
			buzzerStarting(&buzzerCount, &buzzarStart);
		}

  		//if sec is < then 60
  		if(sec < 60)
  			sec++;
  		if(sec == 60)
  		{
  			if(min < 60)
  				min++;
  			sec = 0;
  		}

  		if (min==60)
		{
			if(hor<24)
				hor++;
			min=0;

		}	
		if (hor==24)
		{
			hor=0;
		}

		_delay_ms(150);
  		send_a_command(0x80 + 0); //Clear Screen
  		_delay_ms(5);

	}
}
