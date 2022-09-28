// lab1_code.c 
// R. Traylor
// 7.13.20

//This program increments a binary display of the number of button pushes on switch 
//S0 on the mega128 board.

#include <avr/io.h>
#include <util/delay.h>

//*******************************************************************************
//                            debounce_switch                                  
// Adapted from Ganssel's "Guide to Debouncing"            
// Checks the state of pushbutton S0 It shifts in ones till the button is pushed. 
// Function returns a 1 only once per debounced button push so a debounce and toggle 
// function can be implemented at the same time. Expects active low pushbutton on 
// Port D bit zero. Debounce time is determined by external loop delay times 12. 
//*******************************************************************************
int8_t debounce_s1() {
  static uint16_t state = 0; //holds present state
  state = (state << 1) | (! bit_is_clear(PIND, 0)) | 0xE000;
  if (state == 0xF000) return 1;
  return 0;
}
//*******************************************************************************
// Debouncing for s2. Essentially the same as the code for s1, but checks bit 1 
// instead of bit 0
//*******************************************************************************
int8_t debounce_s2() {
  static uint16_t state = 0; // hold state
  state = (state <<1) | (!bit_is_clear(PIND, 1)) | 0xE000;
   if (state == 0xF000) return 1; 
   return 0;
}

//*******************************************************************************
// Check switch S0 and S1.  When S0 found low for 12 passes of "debounce_switch(), 
//increment PORTB. This will make an incrementing count on the port B LEDS. When 
//S1 found low for 12 passes of debouncing function, decrement the count + display
//on the LEDs 
//*******************************************************************************
int main()
{
DDRB = 0xFF;  //set port B to all outputs
static int port_state = 0;
while(1){     //do forever
 if(debounce_s1()) { //if switch true for 12 passes, increment port B
        PORTB++;
 }
 if(debounce_s2()) { // if switch true for 12 passes, decrement port B
	PORTB--;
 }
  _delay_ms(2);                    //keep in loop to debounce 24ms 
 } //while 
} //main

// The LEDS do not increment/decrement when the buttons are pushed at the same time, however they do flash momentarily. This could possibly
// Be solved with an if statement within the main loop that checks if the buttons are pushed at the same time and resets PORTB to a variable
// holding its current state. However when trying this it seemed to make no difference.
