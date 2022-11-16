// short and sweet test code to mess around with tcnt3 to get it ready for use in the main file

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t counth = 0, countl = 0;
static uint8_t index = 0;
static uint8_t digit = 0;
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

/**************************************************************************
 * Similar in structure to the segsum function, this function is responsible
 * for loading the hour and minute counts onto the seven segment display. 
 * The top two digits (3,2) are for hours, the bottom two (1,0) are for 
 * minutes. Hours and minutes are respectively broken into high and low
 * digits so they can be properly displayed on the LED display. No digits
 * are blanked out, single digit hours will have a leading zero as is common 
 * in 24 hour clock displays
 **************************************************************************/
void update_time(int8_t min, uint8_t hour) {
	//Break up minutes display into high and low bit
	uint8_t min_l = (min % 10);
	uint8_t min_h = ((min/10) % 10);
	//Break up hours display into high and low bit
	uint8_t hr_l = (hour % 10);
	uint8_t hr_h = ((hour/10) % 10);

	//send hours/mins to segments
	segment_data[0] = dec_to_7seg[min_l];
	segment_data[1] = dec_to_7seg[min_h];
	segment_data[3] = dec_to_7seg[hr_l];
	segment_data[4] = dec_to_7seg[hr_h];
}

/**************************************************************************
 * Code for debouncing pushbuttons adapted from Ganssele's guide to 
 * debouncing
 **************************************************************************/
uint8_t chk_buttons(uint8_t button) {
  static uint8_t pbstate[8] = {0};  // create initialize button state storage array with zeros
  pbstate[button] = (pbstate[button] << 1) | (! bit_is_clear(PINA, button)) | 0xE000;
  if(pbstate[button] == 0xF000) return 1;
  return 0;
}

void io_timer_init() {
	TCCR3A = 0x00; //set to normal mode
        TCCR3B = (1<<CS31) | (1<<CS30); //64 prescaler
        TCCR3C = 0x00; //no forced compare

	ETIMSK |= (1<<TOIE3);
}

//ISRS
ISR(TIMER3_OVF_vect){
	DDRA = 0x00;  // set PORTA to all inputs
        PORTA = 0xFF; // enable all pullup resistors on PORTA
        PORTB |= (1<<PB4) | (1<<PB5) | (1<<PB6); //enable tristates
	if(chk_buttons(index)){
		countl++;
		counth++;
	}
	PORTB &= ~(1<<PB6); //disable tristates once buttons are checked
	if(index > 8) index = 0;
        else{
	    index++;
	}

	update_time(countl, counth); //update the display array

	DDRA = 0xFF;
	PORTB &= 0x8F; //clear
	PORTA = segment_data[digit];
	PORTB = (digit << 4);

	if(digit == 5) digit = 0;
	else {
		digit++;
	}

	//get faster clock
	TCNT3H = 0xFF;
	TCNT3L = 0x00;
}

void main() {
    //Initialize display array
    segment_data[0] = dec_to_7seg[counth];
    segment_data[1] = dec_to_7seg[counth];
    segment_data[2] = 0xDF; //just colon on initially
    segment_data[3] = dec_to_7seg[countl];
    segment_data[4] = dec_to_7seg[countl];

    DDRB |= (1<<PB4) | (1<<PB5) | (1<<PB6) | (1<<PB7); //set port bits 4-7 B as outputs [0b11110000]
    io_timer_init();
    sei(); //interrupts on
    while(1) {
    }
}
