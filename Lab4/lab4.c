/************************************************************************
 * Lab 4: Alarm Clock
 * K. Kopcho
 * 11/14/2022
*************************************************************************/
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//define globals
uint8_t sec_count = 0; //counter for how many seconds are elapsed
uint8_t	min_count = 0; //counter for how many minutes have elapsed
uint8_t hour_count = 0; //counter for storing elapsed hour
uint8_t alarm_hour = 0; //stores user set hour for alarm setting
uint8_t alarm_min = 0; //stores user set min for alarm setting
uint8_t armed = 0; //toggle for if alarm is armed
uint8_t sound_alarm = 0; //flag for actually sounding the alarm
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

/************************************************************************
 * Starts TCNT0. Initialization for the external oscillator based on the 
 * demo code for the bargraph
 ************************************************************************/
void clock_init() {
    //start the timer
    ASSR   |=  (1<<AS0); //ext osc TOSC
    TIMSK |= (1 << TOIE0); //enable interrupts on timer 0
    TCCR0 = (1 << CS00) | (1 << CS02); //set prescaler to 128 so ovflw every 1s

    //Initialize display
    segment_data[0] = dec_to_7seg[min_count];
    segment_data[1] = dec_to_7seg[min_count];
    segment_data[2] = 0xFF;
    segment_data[3] = dec_to_7seg[hour_count];
    segment_data[4] = dec_to_7seg[hour_count];
}

/************************************************************************
 * Starts TCNT1, the oscillator responsible for sounding the timer. The 
 * TCNT1 register controls the frequency at which the alarm is sounded.
 ************************************************************************/
void alarm_init() {
	TCNT1 = 20000;
	TIMSK |= (1<<TOIE1); //enable tcnt1 interrupt
	TCCR1A = 0x00; //normal mode
	TCCR1B = (1<<CS10); //no prescale
}

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
  static uint16_t pbstate[8] = {0};  // create initialize button state storage array with zeros
  pbstate[button] = (pbstate[button] << 1) | (! bit_is_clear(PINA, button)) | 0xE000;
  if(pbstate[button] == 0xF000) return 1;
  return 0;
}

/**************************************************************************
 * Function responsible for setting the time of the clock. Called when time
 * set flag is toggled by button 0. Within this function, button 1 toggles
 * whether or not the user is setting the hours or the minutes, button
 * 2 toggles changing the minutes or hours by 1 or by 10, button 3 increments
 * and button 4 decrements
 **************************************************************************/
void set_time(){
	uint8_t static toggletime = 0; //sets whether or not we're incrementing hours or minutes
	uint8_t static togglecnt = 0; //toggle inc by 1 (0) or by 10 (1)
	//check buttons + set flags
	if(chk_buttons(1)){
		toggletime ^= 1; //flip bit for changing between hours and mins
	}
	if(chk_buttons(2)){
		togglecnt ^= 1; //cnt by 1 or cnt by 10
	}
	if(chk_buttons(3)){
		switch(toggletime){
			case 0: //if we're working with minutes
			     if(min_count >= 60 && min_count < 65){ min_count = 0; hour_count++;} //rollover
			     else if(min_count < 60) {
			          if(togglecnt == 0){ //if 0: normal op, inc by 1
	                               min_count++;
                                  }
                                  else if(togglecnt == 1){ //if 1: inc by 10
	                               min_count += 10;
                                  }
			     }
			     break;
			case 1:
			     if(hour_count >= 24 && hour_count < 29) {hour_count = 0;} //rollover
			     else if(hour_count < 24){
				   if(togglecnt == 0){ //if 0: normal op, inc by 1
				 	hour_count++;
				   }
				   else if(togglecnt == 1){ //if 1: inc by 10
					hour_count += 10;
				   }
			     }
			break;
			default: break;
		}
	}
	if(chk_buttons(4)){
		switch(toggletime){
		        case 0:
			     if(min_count > 59 && min_count > 64) min_count = 59; //underflow to 60
			     else if (min_count >= 0 || min_count < 60){
				  if(togglecnt == 0){ //if 0: normal op, dec by 1
	                               min_count--;
                                  }
                                  else if(togglecnt == 1){ //if 1: dec by 10
	                               min_count -= 10;
                                  }
			     }
			     break;
			case 1:
			     if(hour_count > 23 && hour_count > 28) hour_count = 23; //underflow to 23 
			     else if(hour_count >= 0 || hour_count < 24){
				  if(togglecnt == 0){ //if 0: normal op, dec by 1
	                               hour_count--;
                                  }
                                  else if(togglecnt == 1){ //if 1: dec by 10
	                               min_count -= 10;
                                  }
			     }
			     break;
			default: break;
		}
	}
}


/**************************************************************************
 * Function responsible for setting the alarm. Identical to time set
 * function, except it modifies the alarm globals. Admittedly, the alarm
 * and time set functions should be generics, but for some reason when 
 * passing globals into a generic function it doesn't actually update them
 * (probably because variables passed into a function just become copies
 * local to that function.)
 **************************************************************************/
void set_alarm(){
	uint8_t static toggletime = 0; //sets whether or not we're incrementing hours or minutes
	uint8_t static togglecnt = 0; //toggle inc by 1 (0) or by 10 (1)
	//check buttons + set flags
	if(chk_buttons(1)){
		toggletime ^= 1; //flip bit for changing between hours and mins
	}
	if(chk_buttons(2)){
		togglecnt ^= 1; //cnt by 1 or cnt by 10
	}
	if(chk_buttons(3)){
		switch(toggletime){
			case 0: //if we're working with minutes
			     if(alarm_min >= 60 && alarm_min < 65){alarm_min = 0; alarm_hour++;} //rollover
			     else if(alarm_min < 60) {
			          if(togglecnt == 0){ //if 0: normal op, inc by 1
	                               alarm_min++;
                                  }
                                  else if(togglecnt == 1){ //if 1: inc by 10
	                               alarm_min += 10;
                                  }
			     }
			     break;
			case 1:
			     if(alarm_hour >= 24 && alarm_hour < 29) {alarm_hour = 0;} //rollover
			     else if(alarm_hour < 24){
				   if(togglecnt == 0){ //if 0: normal op, inc by 1
				 	alarm_hour++;
				   }
				   else if(togglecnt == 1){ //if 1: inc by 10
					alarm_hour += 10;
				   }
			     }
			break;
			default: break;
		}
	}
	if(chk_buttons(4)){
		switch(toggletime){
		        case 0:
			     if(alarm_min > 59 && alarm_min > 64) alarm_min = 59; //underflow to 60
			     else if (alarm_min >= 0 || alarm_min < 60){
				  if(togglecnt == 0){ //if 0: normal op, dec by 1
	                               alarm_min--;
                                  }
                                  else if(togglecnt == 1){ //if 1: dec by 10
	                               alarm_min -= 10;
                                  }
			     }
			     break;
			case 1:
			     if(alarm_hour > 23 && alarm_hour > 28) alarm_hour = 23; //underflow to 23 
			     else if(alarm_hour >= 0 || alarm_hour < 24){
				  if(togglecnt == 0){ //if 0: normal op, dec by 1
	                               alarm_hour--;
                                  }
                                  else if(togglecnt == 1){ //if 1: dec by 10
	                               alarm_min -= 10;
                                  }
			     }
			     break;
			default: break;
		}
	}
}


//ISRS
 /**************************************************************************
 * ISR for TCNT0. Takes care of updating the counters for seconds, minutes
 * and hours, and then updating the display arrays. It also checks whether
 * or not the alarm settings match the current time.
 * This interrupt happens precisely every 1 second due to the external 
 * 32KHz oscillator being prescaled by 128. 
 **************************************************************************/
ISR(TIMER0_OVF_vect){
	//RTC
	if(sec_count < 60) { //if less than 60s have elapsed
		sec_count++;
	}
	if(sec_count == 60){ //if a min has passed
		sec_count = 0; //reset seconds for next minute
		if(min_count < 60) {
		    min_count++; //increment minutes
		}
	}
	if(min_count == 60) { //if an hour has passed
		min_count = 0; //reset min count for next hour
		if(hour_count < 24) {
			hour_count++; //increment hours
		}
	}
	if(hour_count == 24){
		//Reset hour counter
		hour_count = 0; //reset hour_count for next 24 hours;
	}

	if(armed){
		if(segment_data[2] == 0xFF){ //if both off
			segment_data[2] = 0xF8; //turn on colon and indicator
		}
		else if(segment_data[2] == 0x04){ //if colon on and indicator off
			segment_data[2] = 0xFB; //turn colon off and indicator on
		}
		else if(segment_data[2] == 0xF8){ //if both on
			segment_data[2] = 0xFB; //turn colon off, leave indicator on
		}
		else if(segment_data[2] == 0xFB){ //if colon off and indicator on
			segment_data[2] = 0xF8; //turn on both
		}
	}
	else{
		if(segment_data[2] == 0xF8 || segment_data[2] == 0xFB){ //clear on return
			segment_data[2] = 0xFF;
		}
		if(segment_data[2] == 0xFF){ //if off, turn on
			segment_data[2] = 0xFC;
		}
		else if(segment_data[2] == 0xFC) { //if on, turn off
			segment_data[2] = 0xFF;
		}
	}
}

/**************************************************************************
 * ISR for TCNT1. Handles sounding the alarm when a flag is set. 
 * Otherwise, if the flag isn't set, this just gets passed over
 **************************************************************************/
ISR(TIMER1_OVF_vect){
	if(sound_alarm == 1){
		PORTC ^= (1 << PC0); //toggle output pin
		TCNT1 = 20000; //reset overflow value
	}
}

uint8_t main() {
    DDRB |= (1<<PB4) | (1<<PB5) | (1<<PB6) | (1<<PB7); //set port bits 4-7 B as outputs [0b11110000]
    static uint8_t settime = 0;
    static uint8_t setalarm = 0;
    clock_init(); //start RTC
    alarm_init(); //start oscillator for alarm
    sei(); //interrupts on

    while(1){
	_delay_us(250);
	DDRA = 0x00;
	PORTA = 0xFF;
	PORTB |= (1<<PB4) | (1<<PB5) | (1<<PB6);
	DDRA = 0xFF;

	for(int x = 0; x < 8; x++) {
		if(chk_buttons(0)){ //button that enters/exits time setting mode
		     TIMSK ^= (1<<TOIE0); //stop the interrupts while clock settings are changed
		     sec_count = 0; //reset seconds count
		     settime ^=1;
		     break;
		}
		if(chk_buttons(6)){ //button to stop alarm
		     sound_alarm = 0; //stop alarm sounding
		     armed = 0; //clear armed flag
		     break;
		}
		if(chk_buttons(7)){ //button that enters/exits alarm setting mode
		     setalarm ^= 1;
		     break;
		}
	}

	if(settime && !setalarm){
		set_time(); //go through time setting ui
	}
	if(setalarm && !settime){
		armed = 1; //mark the alarm as armed
		set_alarm(); //go through alarm setting UI
		update_time(alarm_min, alarm_hour); //update display to show alarm settings
	}
	if(setalarm && settime){ //don't let the time set and alarm set functions run simulataneously
		setalarm = 0;
		settime = 0;
	}

	if(!setalarm){
	    update_time(min_count, hour_count); //update display like normal
	}

	//push update to 7seg display
        PORTB &= ~(1<<PB6);
	for(int i = 0; i < 5; i++) {
            PORTB = (i << 4);
            PORTA = segment_data[i];
	    _delay_us(250);
       }

    }
}
