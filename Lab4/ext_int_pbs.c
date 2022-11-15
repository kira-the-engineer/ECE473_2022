//Quick and scrappy test code for figuring out if the pushbutton board can trigger external interrupts

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

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

uint8_t seg_sum = 0;
//***********************************************************************************
//                                   segment_sum                                    
//takes a 16-bit binary input value and places the appropriate equivalent 4 digit 
//BCD segment code in the array segment_data for display.                       
//array is loaded at exit as:  |digit3|digit2|colon|digit1|digit0|
void segsum(uint16_t sum) {
  //determine how many digits there are 
  uint8_t dig_cnt; 
  if(sum < 10) dig_cnt = 1; // checks if sum is 0 through 9
  else if(sum < 100 && sum >= 10) dig_cnt = 2; // checks if sum is 10 through 99
  else if(sum < 1000 && sum >= 100) dig_cnt = 3; // checks if sum is 1000 through 999
  else if(sum > 99 && sum <= 1023) dig_cnt = 4; // checks if sum is 1000 through 1023
  else {
       dig_cnt = 0; // if sum is greater than 1023 or error
  }

  //break up decimal sum into 4 digit-segments
  //create integers to break sum value into ones, tens, hundreds, thousands place
  uint8_t ones = (sum % 10);
  uint8_t tens = ((sum / 10) % 10);
  uint8_t hundreds = ((sum / 100) % 10);
  uint8_t thousands = ((sum / 1000) % 10);

  //load values into segment data array
  segment_data[0] = dec_to_7seg[ones]; // get digit for ones place
  segment_data[1] = dec_to_7seg[tens]; // get digit for tens place
  segment_data[2] = 0xFF; // Keep colon off
  segment_data[3] = dec_to_7seg[hundreds]; // get digit for hundreds place
  segment_data[4] = dec_to_7seg[thousands]; // get digit for thousands place


  //blank out leading zero digits
  switch(dig_cnt){
	case 4: segment_data[4] = dec_to_7seg[1]; break; // if there's 4 digits, don't blank anything and break
	case 3: segment_data[4] = 0xFF; break; // if there's 3 digits, blank the thou place
  	case 2: segment_data[4] = 0xFF; segment_data[3] = 0xFF; break; // if there's 2 digits, blank thou, hund place 
	case 1: segment_data[4] = 0xFF; segment_data[3] = 0xFF; segment_data[1] = 0xFF; break; // if there's 1 digit blank thou, hund, tens place
	//just display a single zero if 0 or over 1023
	default: segment_data[4] = 0xFF; segment_data[3] = 0xFF; segment_data[1] = 0xFF; segment_data[0] = 0x00; break; 
  }
}//segment_sum
//***********************************************************************************

//******************************************************************************
//                            chk_buttons                                      
//Checks the state of the button number passed to it. It shifts in ones till   
//the button is pushed. Function returns a 1 only once per debounced button    
//push so a debounce and toggle function can be implemented at the same time.  
//Adapted to check all buttons from Ganssel's "Guide to Debouncing"            
//Expects active low pushbuttons on PINA port.  Debounce time is determined by 
//external loop delay times 12. 
//
uint8_t chk_buttons(uint8_t button) {
//******************************************************************************
  static uint16_t pbstate[8] = {0};  // create initialize button state storage array with zeros
  pbstate[button] = (pbstate[button] << 1) | (! bit_is_clear(PINA, button)) | 0xE000;
  if(pbstate[button] == 0xF000) return 1;
  return 0;
}

ISR(INT4_vect){
    if(seg_sum <= 1023){
	seg_sum++;
    }
    else{
	seg_sum = 0;
    }

    PORTC ^= (1<<PC0); //Reset bit before exiting interrupt so we only need a single button press to trigger it
}

uint8_t main() {
    DDRB |= (1<<PB4) | (1<<PB5) | (1<<PB6) | (1<<PB7); //set port bits 4-7 B as outputs [0b11110000]
    DDRC |= (1<<PC0); //set port C bit zero as an output
    DDRE |= 0x00; //set interrupt INT4 pin to input

    //initialize interrupt
    EIMSK |= (1<<INT4); //enable external interrupt 4
    EICRB |= (1<<ISC41) | (0<<ISC40); //set interrupt to trigger on falling edge

    sei(); //enable interrupts
    while(1) {
	_delay_ms(2); // keep in loop for 24ms (12 shifts * 2)
        DDRA = 0x00;  // set PORTA to all inputs
        PORTA = 0xFF; // enable all pullup resistors on PORTA
	PORTB |= (1<<PB4) | (1<<PB5) | (1<<PB6);
	for(int i=0; i < 8; i++) {
		if(chk_buttons(0)){
			PORTC ^= (1<<PC0);
		}
	}
	PORTB &= ~(1<<PB6); //disable tristate
	segsum(seg_sum);
  	DDRA = 0xFF;
  	for(int i = 0; i < 5; i++) { //update display
        	PORTB = (i << 4);
        	PORTA = segment_data[i];
		_delay_ms(2);
  	}
    }
}
