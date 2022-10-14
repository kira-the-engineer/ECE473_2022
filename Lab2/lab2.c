// lab2.c 
// Modified version of Roger's skeleton code
// K. Kopcho
// 10.12.22

//  HARDWARE SETUP:
//  PORTA is connected to the segments of the LED display. and to the pushbuttons.
//  PORTA.0 corresponds to segment a, PORTA.1 corresponds to segement b, etc.
//  PORTB bits 4-6 go to a,b,c inputs of the 74HC138.
//  PORTB bit 7 goes to the PWM transistor base.

#define TRUE 1
#define FALSE 0
#include <avr/io.h>
#include <util/delay.h>

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
//***********************************************************************************
//                                   segment_sum                                    
//takes a 16-bit binary input value and places the appropriate equivalent 4 digit 
//BCD segment code in the array segment_data for display.                       
//array is loaded at exit as:  |digit3|digit2|colon|digit1|digit0|
void segsum(uint16_t sum) {
  //determine how many digits there are 
  uint8_t dig_cnt = 0; 
  if(sum < 10) dig_cnt = 1; // checks if sum is 0 through 9
  else if(sum < 100 && sum >= 10) dig_cnt = 2; // checks if sum is 10 through 99
  else if(sum < 1000 && sum >= 100) dig_cnt = 3; // checks if sum is 1000 through 999
  else if(sum <= 1023) dig_cnt = 4; // checks if sum is 1000 through 1023
  else {
       dig_cnt = 0; // if sum is greater than 1023 or error
  }

  //break up decimal sum into 4 digit-segments
  //create integers to break sum value into ones, tens, hundreds, thousands place
  uint8_t ones = (sum % 10);
  uint8_t tens = ((sum / 10) % 10);
  uint8_t hundreds = ((sum / 100) % 10);
  uint8_t thousands = ((sum / 1000) % 10);

  //load encoded values into segment data array
  segment_data[0] = dec_to_7seg[ones]; // encode digit for ones place
  segment_data[1] = dec_to_7seg[tens]; // encode digit for tens place
  segment_data[2] = 0xFF; // Keep colon off
  segment_data[3] = dec_to_7seg[hundreds]; // encode digit for hundreds place
  segment_data[4] = dec_to_7seg[thousands]; // encode digit for thousands place


  //blank out leading zero digits
  switch(dig_cnt){
	case 4: break; // if there's 4 digits, don't blank anything and break
	case 3: segment_data[4] = 0xFF; break; // if there's 3 digits, blank the thou place
  	case 2: segment_data[4] = 0xFF; segment_data[3] = 0xFF; break; // if there's 2 digits, blank thou, hund place 
	case 1: segment_data[4] = 0xFF; segment_data[3] = 0xFF; segment_data[1] = 0xFF; break; // if there's 1 digit blank thou, hund, tens place
	//just display a single zero if 0 or over 1023
	default: segment_data[4] = 0xFF; segment_data[3] = 0xFF; segment_data[1] = 0xFF; segment_data[0] = 0x00; break; 
  }
}//segment_sum
//***********************************************************************************


//***********************************************************************************
uint8_t main()
{
uint16_t seg_sum = 0; // create variable to count from pushbuttons
DDRB |= (1<<PB4) | (1<<PB5) | (1<<PB6); //set port bits 4-7 B as outputs [0b11110000]
while(1){
  _delay_ms(2); // keep in loop for 24ms (12 shifts * 2)
  DDRA = 0x00;  // set PORTA to all inputs
  PORTA = 0xFF; // enable all pullup resistors on PORTA
  // enable tristate buffer for pushbutton switches by setting seg0, seg1, seg2 high.
  // this puts all outputs of the 8 bit decoder to high except for bit 8. Since the
  // seven segment display is active low, this ensures the pushbuttons on port A
  // won't blank out the segments. Bit 8 is tied to the enable pin of the tristate
  // buffer on the pushbutton board, and drives the enable pin
  for(int i = 0; i < 8; i++){ // iterate over buttons on PORTA 
	if(chk_buttons(i)) {
		seg_sum += (1 << i); // Use binary shifting of 0b00000000 to make 
				     // each switch increment the value of sum by a power of 2
	}
  }
  //disable tristate buffer for pushbutton switches by clearing bit 4 (Seg 2), sending bit 8 of the decoder low
  DDRB &= ~(1<<PB4);
  //bound the count to 0 - 1023
  if(seg_sum > 1023) {
	seg_sum = 0;
  }
  //break up the disp_value to 4, BCD digits in the array: call (segsum)
  segsum(seg_sum); 
  //make PORTA an output
  DDRA = 0xFF;
  for(int i = 0; i < 5; i++) {
	PORTB = (i << 4);
	PORTA = segment_data[i];
	_delay_ms(2);
  }
	// Pseudocode below here:
        // Need to shift which segmment select pins are driven
        // because we don't want to overwrite every digit each time
	// Seg2 = PB4, Seg1 = PB5, Seg0 = PB6
        // Digit 4 [ones] (Y0) needs to be on first- which means Seg0, Seg1, Seg2 need to be low [0x00]
	// Digit 3 [tens] (Y1) needs to be on next - Seg0 = H, Seg1 = H, Seg2 = L [0x10]
	// Digit 2 [colon] (Y2) needs to be on next - Seg0 = L, Seg1 = H, Seg2 = L [0x20]
	// Digit 1 [hunds] (Y3) needs to be on next - Seg0 = L, Seg1 = H, Seg2 = H [0x30]
	// Digit 0 [thou] (Y4) needs to be on next - Seg0 = H, Seg1 = L, Seg2 = L [0x40]
	// To do this in a loop, we can binary shift left by 4: (0 = 0, 1 = 0x10, 2 = 0x20, 3 = 0x30, 4 = 0x40)
  }//while
}//main
