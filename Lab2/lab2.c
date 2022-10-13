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
uint8_t segment_data[5] 

//decimal to 7-segment LED display encodings, logic "0" turns on segment
uint8_t dec_to_7seg[12] 


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
  //break up decimal sum into 4 digit-segments
  //blank out leading zero digits 
  //now move data to right place for misplaced colon position
}//segment_sum
//***********************************************************************************


//***********************************************************************************
uint8_t main()
{
uint16_t seg_sum; // create variable to count from pushbuttons
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
		seg_sum += (1 << i) // Use binary shifting of 0b00000000 to make 
				    // each switch increment the value of sum by a power of 2
	}
  }
  //disable tristate buffer for pushbutton switches by clearing bit 6, sending bit 8 low
  PORTB &= ~(1<<PB6);
  //bound the count to 0 - 1023
  if(seg_sum > 1023) {
	seg_sum = 0;
  }
  //break up the disp_value to 4, BCD digits in the array: call (segsum)
  //bound a counter (0-4) to keep track of digit to display 
  //make PORTA an output
  //send 7 segment code to LED segments
  //send PORTB the digit to display
  //update digit to display
  }//while
}//main
