// testpb.c
// K. Kopcho
// 10.11.2022

//This program tests the function of the pushbutton breakout board by lighting
//Leds on each button press

#include <avr/io.h>
#include <util/delay.h>

//*******************************************************************************
//                            debounce_switch                                  
// Adapted from Ganssel's "Guide to Debouncing"            
// Checks the state of pushbutton S0 It shifts in ones till the button is pushed. 
// Function returns a 1 only once per debounced button push so a debounce and toggle 
// function can be implemented at the same time. Expects active low pushbutton on 
// Port A. Change bit number in the "bit_is_clear" call to change which pushbutton
// is tested. Debounce time is determined by external loop delay times 12. 
//*******************************************************************************
int8_t debounce() {
  static uint16_t state = 0; //holds present state
  state = (state << 1) | (! bit_is_clear(PINA, 0)) | 0xE000;
  if (state == 0xF000) return 1;
  return 0;
}

//*******************************************************************************
// Check switches  When switch found low for 12 passes of "debounce(), 
//increment PORTB. This will make an incrementing count on the port B LEDS.
//*******************************************************************************
int main()
{
DDRB = 0xFF;  //set port B to all outputs
DDRA = 0x00;  //set port A to all inputs
PORTA = 0xFF; //set port A to all pullups
while(1){     //do forever
 if(debounce()) { //if switch true for 12 passes, increment port B
        PORTB++;
 }
  _delay_ms(2);                    //keep in loop to debounce 24ms 
 } //while 
} //main

