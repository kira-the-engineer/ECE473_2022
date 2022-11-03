//lab 3
// K. Kopcho

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


//Bring over segment array defs from Lab2
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
//			       spi_init
//Initialization function for the SPI ports on the atmega128. Based around the
//demos in the SPI lecture as well as the skeleton code for the bargraph demo
//Sets up ports for SS, MOSI, SCLK. MISO (PB3) is an input, so it doesn't need
//to be pulled high
//
********************************************************************************/
void spi_init(void) {
    DDRB  |=   (1<<PB0) | (1<<PB1) | (1<<PB2); //Turn on SS, MOSI, SCLK
    SPCR  =   (1<<SPE) | (1<<MSTR) | (0<<CPHA) | (1<<CPOL);  //enable SPI, master mode 
}

//******************************************************************************
//			      timer_init
//Initializes the timer to run off the i/o clock, in normal mode with a 128
//prescaler. Used for generating interrupts to update encoder and buttons
//
********************************************************************************/
void timer_init(void) {
     TIMSK |= (1<<TOIE0);             //enable interrupts
     TCCR0 |= (1<<CS02) | (1<<CS00);  //normal mode, prescale by 128
}

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

uint8_t main() {
  // Begin port initialization
  DDRB |= (1<<PB4) | (1<<PB5) | (1<<PB6) | (1<<PB7); //Set Port B 4-7 as output
  DDRD |= (1<<PD2) | (1<<PD3); //Set PD2, PD3 as outputs for controlling Regclk and OE on 595
  DDRE |= (1<<PE5) | (1<<PE6); //Set PD5, PD6 as outputs for CLK inhibit and Shift/Load on 165
  //PORTA's status as either an output/input should be modified within the main while/by the interrupt

  //Initialize timer and SPI interface
  spi_init();
  timer_init();

  //Initialize PORTE Values for encoder register control
  PORTE |= (1<<PE5); //Set clock inhibit high
  PORTE |= (0<<PE6); //Pull shift ld low

  sei(); //enable global interrupts

  while(1){




  }

}
