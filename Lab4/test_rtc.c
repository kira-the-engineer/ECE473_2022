/************************************************************************
 * Code for setting up and testing a real time counter/clock using 
 * TCNT0. 
 * Author: K. Kopcho
 * Date: 11/14/2022
 *
 ************************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//define globals
uint8_t sec_count = 0; //counter for how many seconds are elapsed
uint8_t	min_count = 0; //counter for how many minutes have elapsed
uint8_t hour_count = 0;
//holds data to be sent to the segments. logic zero turns segment on
uint8_t segment_data[5]; 

//decimal to 7-segment LED display encodings, logic "0" turns on segment
//used in the decimal to segment encoding function
uint8_t dec_to_7seg[12] = {
       0b11000000, // display 0
       0b11111001, // display 1
       0b10100100, // display 2
       0b10110000, // display 3
       0b10011001, // display 4
       0b10010010, // display 5
       0b10000010, // display 6 
       0b11111000, // display 7
       0b10000000, // display 8
       0b10010000, // display 9
       0b01111111, // display . 
       0b11111111, // clear all segments (active low)

};

/************************************************************************
 * Starts TCNT0. Initialization for the external oscillator based on the 
 * demo code for the bargraph
 ************************************************************************/
void clock_init() {
    //start the timer
    ASSR   |=  (1<<AS0); //ext osc TOSC
    TIMSK |= (1 << TOIE0); //enable interrupts on timer 0
    TCCR0 = (1 << CS00) | (1 << CS02); //set prescaler to 128 so ovflw every 1s

    //Initialize display
    segment_data[0] = dec_to_7seg[0];
    segment_data[1] = dec_to_7seg[0];
    segment_data[2] = 0xFF; //on initially
    segment_data[3] = dec_to_7seg[0];
    segment_data[4] = dec_to_7seg[0];
}

/**************************************************************************
 * Similar in structure to the segsum function, this function is responsible
 * for loading the hour and minute counts onto the seven segment display. 
 * The top two digits are for hours, the bottom two are for minutes
 **************************************************************************/
void update_time(int8_t min, uint8_t hour) {
	

}

//ISRS
/**************************************************************************
 * ISR for TCNT0. Takes care of updating the counters for seconds, minutes
 * and hours, and then updating the actual display with those values.
 * This interrupt happens precisely every 1 second due to the external 
 * 32KHz oscillator being prescaled by 128. 
 **************************************************************************/
ISR(TIMER0_OVF_vect){
	if(sec_count < 60) { //if less than 60s have elapsed
		sec_count++;
	}
	if(sec_count == 60){ //if a min has passed
		sec_count = 0; //reset seconds for next minute
		if(min_count < 60) {
		    min_count++; //increment minutes
		}
	}
	if(min_count == 60) { //if an hour has passed
		min_count = 0; //reset min count for next hour
		if(hour_count < 24) {
			hour_count++; //increment hours
		}
	}
	if(hour_count == 24){
		//Reset all counters
		hour_count = 0;
		min_count = 0;
	 	sec_count = 0;
	        //Reset display to 00:00
		segment_data[0] = dec_to_7seg[0];
                segment_data[1] = dec_to_7seg[0];
                segment_data[3] = dec_to_7seg[0];
                segment_data[4] = dec_to_7seg[0];
	}
	//toggle colon on and off every 1s
	segment_data[2] ^= 0xFF;
}


uint8_t main() {
    DDRB |= (1<<PB4) | (1<<PB5) | (1<<PB6) | (1<<PB7); //set port bits 4-7 B as outputs [0b11110000]
    clock_init(); //start RTC
    sei(); //interrupts on

    while(1){
	DDRA = 0xFF;
	for(int i = 0; i < 5; i++) {
            PORTB = (i << 4);
            PORTA = segment_data[i];
	    _delay_ms(2);
       }
    }
}
