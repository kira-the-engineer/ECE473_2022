// cap_meter.c

// Code to implement capacitance meter with Mega128.
// Implemented as integrating quantizer.

//compute a multiplicitive factor to make code cleaner. This is
//simply a cranking of the equation for current through a cap
//given the dv, Ic, and bandgap reference voltage.
#define DtoC_FACTOR 0.2340    //5V wall wart supply for accuracy

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "hd44780.h"
#include <string.h>
#include <stdio.h>

//global variable with LCD text, so ISR can change it
char lcd_message[14] = {"xxxx.x nF cap"};
/*****************************************************************************/
//SPI initalization for LCD display
void spi_init(void){
//stolen from the adc in class assignment
 DDRB  |= 0x07;  //Turn on SS_n, MOSI, SCLK. SS_n must be out for MSTR mode 

 //Master mode, Clock=clk/2, Cycle half phase, Low polarity, MSB first
 SPCR=(1<<SPE) | (1<<MSTR); //enable SPI, clk low initially, rising edge sample
 SPSR=(1<<SPI2X);           //SPI at 2x speed (8 MHz)
}
/*****************************************************************************/

/*****************************************************************************/
//Comparator init
//Use bandgap reference (1.23V), Input capture trigger for TCNT1 enabled 
void acomp_init(void){
	ACSR |= (1<<ACIC) | (1<<ACBG); //set bandgap ref and allow interrupt on TC1 to be triggered
}
/*****************************************************************************/

/*****************************************************************************/
//TCNT3_init
//Run in normal mode. Used to set the refresh rate for the LCD display.
//Refresh rate shold be about 4mS. Does not generate interrupts. Poll for
//interrupt flag.
void tcnt3_init(void){
	TCCR3A = 0x00; //normal mode
	TCCR3B |= (1<<CS10) | (1<<CS11); //1024 prescale
	TCCR3C = 0x00; //no forced compare
}
/*****************************************************************************/

/*****************************************************************************/
//TCNT1_init
//Use in normal mode. Suggest no pre-scaler. Start up initially with counter off.
//Must enable TCNT1 input capture and overflow interrupts.
//
void tcnt1_init(void){
	TCCR1A = 0x00; //normal mode
	TCCR1B = 0x00; //no prescaler, timer stopped initially
	TCCR1C = 0x00; //no forced compare
	TIMSK = (1<<TOIE1) | (1<<TICIE1); //enable interrupts on timer counter port
}
/*****************************************************************************/

/*****************************************************************************/
//Input capture register ISR
//When input capture register interrupt happens: 
//  - read counter 1 value
//  - disable counter
//  - convert to cap valu ~500uA / dv/dt.
//  - fill in the LCD message string only 
//
ISR(TIMER1_CAPT_vect){
	int dt = TCNT1; //save counter 1 val as dt (time to charge cap)
	TCCR1B = 0x00; //stop timer
	int capval = 0.000571 / (1.23 / dt); // C = 571uA / (dv/dt)
	//leading 0 supression here
	char capstr[7]; //create string large enough to store "xxxx.x" cap val
	sprintf(capstr, "%d", capval); //format int to string
	strncpy(lcd_message, capstr, 6); //send to lcd_message;
	strcat(lcd_message, " nF cap");
}
/*****************************************************************************/

/*****************************************************************************/
//                              timer1 overflow ISR
//If TCNT1 overflows, before the analog comparator triggers, disable counter
// and display "---.-" to LCD
//
ISR(TIMER1_OVF_vect){
	TCCR1B = 0x00; //stop timer
	TCNT1 = 0x00; //set back to zero
	strcpy(lcd_message, "----.-");
}
/*****************************************************************************/

/*****************************************************************************/
int main(){
    //setup PORTB.0 LED for blinking
    DDRB |= 0x01; //Port B bit 0 as output
    //setup PORTF.3 to clock LCD
    DDRF  |= 0x08;
    PORTF &= 0xF7;  //port F bit 3 is initially low
    //set PE2,3 appropriately
    DDRE |= (0<<PE3); //make PE3 input initially (PE2 gets disconnected by the ABCG being pulled high, gnd if needed)
    spi_init(); //initalize SPI
    tcnt1_init(); //initialize counter/timer one
    tcnt3_init(); //initialize counter/timer three
    acomp_init(); //initialize analog comparator
    _delay_us(75); //wait enough time for bandgap reference to startup 
    lcd_init(); //initialize the LCD
    sei(); //enable interrupts

    while(1){
        if (ETIFR & (1<<TOV3)){   
            ETIFR |= (1<<TOV3); //clear overflow bit for next measurement
            PORTB ^= (1<<PB0); //toggle B0 to see that the meter is running
            TCNT1 = 0x00; //ensure that TCNT1 starts at zero to time the charge interval
            DDRE |= (1<<PE3); //make PE3 an output to discharge cap
            _delay_ms(2); //delay enough to discharge the cap 
            TCCR1B = (1<<CS10);//start TC1 counter, no prescaling (62.5nS/tick)
            DDRE |= (0 << PE3);//change PE3 back to high-Z (input) to allow charging cap
            string2lcd(lcd_message);//write string to LCD; message is created in the ISR
            cursor_home(); //put the cursor back to home
        }//if
    }//while
}//main
