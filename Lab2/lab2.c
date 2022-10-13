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
DDRB = 0xF0; //set port bits 4-7 B as outputs [0b11110000]
while(1){
  _delay_ms(2); // keep in loop for 24ms (12 shifts * 2)
  DDRA = 0x00;  // set PORTA to all inputs
  PORTA = 0xFF; // enable all pullup resistors on PORTA
  //enable tristate buffer for pushbutton switches
  for(int i = 0; i < 8; i++){ // iterate over buttons on PORTA 
	if(chk_buttons(i)) {
		//increment count
	}
  }
  //disable tristate buffer for pushbutton switches
  //bound the count to 0 - 1023
  //break up the disp_value to 4, BCD digits in the array: call (segsum)
  //bound a counter (0-4) to keep track of digit to display 
  //make PORTA an output
  //send 7 segment code to LED segments
  //send PORTB the digit to display
  //update digit to display
  }//while
}//main
