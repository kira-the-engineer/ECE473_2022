#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//globals
uint8_t step = 0; //step to inc/dec by
uint16_t count = 0; //create variable to store overall count to send to display
uint16_t prevcnt = 0; //create variable to store previous count
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

/*******************************************************************************
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
    SPSR  |=   (1 << SPI2X);                        // double speed operation
}

/************************************************************************************
//                                   segment_sum                                    
//takes a 16-bit binary input value and places the appropriate equivalent 4 digit 
//BCD segment code in the array segment_data for display.                       
//array is loaded at exit as:  |digit3|digit2|colon|digit1|digit0|
//
*************************************************************************************/
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
  segment_data[2] = 0xFF; // keep segment off
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


int8_t get_encoder(uint8_t encoder){
  int return_val = -1; //default return value
  uint8_t newA, newB; //new values for encoder pins
  static uint8_t oldA = 1, oldB = 1; //old values for encoder pins
  static int8_t enc_cnt = 0; //keeps state count

  newA = ((encoder & 0x01) == 0) ? 0 : 1; //get current value of the encoder for state machine
  newB = ((encoder & 0x02) == 0) ? 0 : 1;

  if((newA != oldA) || (newB != oldB)) { //check to see if a change occured
	if((newA == 0) && (newB == 0)) { //if both new states are zero
		if(oldA == 1) enc_cnt++;
		else enc_cnt--;
	}
	else if((newA == 0) && (newB == 0)) { //both states are 10
		if(oldA == 0) enc_cnt++;
		else enc_cnt--;
	}
	else if((newA == 1) && (newB == 1)) { //if at detent pos
		if(oldA == 0){
			if(enc_cnt == 3) return_val = 0; //turn right
		}
		else{
			if(enc_cnt == -3) return_val = 1; //turn left
		}
		enc_cnt = 0; //reset encoder count
	}
	else if((newA == 1) && (newB == 0)){ //if both 01
		if(oldA == 1) enc_cnt++;
		else enc_cnt--;
	}
	oldA = newA; //set vals for next loop
	oldB = newB;
  }
  return return_val;
}

uint8_t main() { //try getting encoder values using polling instead of interrupts
    spi_init(); //initialize spi pins
    DDRB |= (1<<PB4) | (1<<PB5) | (1<<PB6) | (1<<PB7); //Set Port B 4-7 as output
    DDRE |= (1<<PE5) | (1<<PE6); //Set PD5, PD6 as outputs for CLK inhibit and Shift/Load on 165

  while(1){
    prevcnt = count; //set previous count to last count
    PORTE |= (0 << PE5); //disable clock inhibit
    PORTE |= (1 << PE6); //enable shifts
    PORTB &= ~(1<<PB6); //disable tristates once buttons are checked

    //Read encoder values
    SPDR = 0x00; //send dummy val to SPDR to toggle read
    while(bit_is_clear(SPSR,SPIF)){} //Spin until tx is finished
    uint8_t read_enc = SPDR;
    read_enc &= 0x03; //blank out digits 2-7
    uint8_t state = get_encoder(read_enc);

    count = state;
    //bound the count to 0 - 1023
    if(count > 1023 && count <= 1027) {
	count = 0;
    }
    else if(count > 1027) {
	count = 1023;
    }

    segsum(count); //display values based on count value

    //make PORTA an output, send bits to display
    DDRA = 0xFF;
    for(int i = 0; i < 5; i++) {
        PORTB = (i << 4);
        PORTA = segment_data[i];
	_delay_ms(2);
    }
  }

}
