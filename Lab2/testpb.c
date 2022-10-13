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
// Similar to the code for the single pushbutton debounce, except for this code
// operates on an array of saved pushbutton states as outlined in Ganssel's code.
// Each time this function is called, ones are shifted into an index in the state
// array at the button index passed to this function. If the value at that index 
// is 0xF000 after 1s shifting, return 1. This stays in the loop for 12 cycles 
// times a delay in main to get a time delay in ms.
//*******************************************************************************
int8_t debounce(uint8_t bidx) {
  static uint16_t pbstate[8] = {0};  //
  pbstate[bidx] = (pbstate[bidx] << 1) | (! bit_is_clear(PINA, bidx)) | 0xE000;
  if(pbstate[bidx] == 0xF000) return 1;
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
 for(int i = 0; i < 8; i++){
     if(debounce(i)) { //if switch true for 12 passes, increment port B
        PORTB++;
     } //if
  } // for
  _delay_ms(2);                    //keep in loop to debounce 24ms 
 } //while 
} //main

