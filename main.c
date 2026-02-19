#include <avr/io.h>
#include <avr/interrupt.h>
int m = 0, ms = 0, s = 0, digit = 0, stare = 0, mode = 1;
int adc_l = 0, t= 0, r = 0, adc_h = 0; float adc = 0; 
int c_m = 0, c_ms = 0, c_s = 0, h = 11;
int ms_adc = 0, set = 2, dec = 0, digit1 = 0, digit2 = 0, Vin = 0, tmp = 0, T = 0;
int ADCMax = 1023, fMax=20000, fMin=20, freq, icr, TOP = 2000, TOP1 = 3000, value, ms_b = 0, adv = 0;
int alarm_m = 0, alarm_s = 10,  alarm1_h = 11, alarm1_m = 1;
void init_display()
{
	DDRA|=0b00001111;
	DDRC|=0b11111111;
}

void display(char p, char c) {
	PORTA &= 0b11110000;
	PORTC &= 0b00000000;
	switch (c) {
		case 0 :
		PORTC |= 0b00111111;
		break;
		case 1 :
		PORTC |= 0b00000110;
		break;
		case 2 :
		PORTC |= 0b01011011;
		break;
		case 3 :
		PORTC |= 0b01001111;
		break;
		case 4 :
		PORTC |= 0b01100110;
		break;
		case 5 :
		PORTC |= 0b01101101;
		break;
		case 6 :
		PORTC |= 0b01111101;
		break;
		case 7 :
		PORTC |= 0b00000111;
		break;
		case 8 :
		PORTC |= 0b01111111;
		break;
		case 9 :
		PORTC |= 0b01101111;
		break;
		case 10 :
		PORTC |= 0b00111001; 
		break; 
		case 11 : 
		PORTC |= 0b01100011;
		break;
	}
	switch(p) {
		case 1:
		PORTA |= 0b00000001;
		break;
		case 2:
		PORTA |= 0b00000010;PORTC |= 0b10000000;
		break;
		case 3:
		PORTA |= 0b00000100;
		
		break;
		case 4:
		PORTA |= 0b00001000;
		break;
	}
	
}

void init_Buzzer(){
	
	TCCR1A = 0b10100010;
	TCCR1B = 0b00011010;
}

void init_timer(){
	SREG = 1<<7;				//global interrupt enable
	TCCR0 = 0b00001011;			//in WGM01:0 am setat pe modul CTC clear time on compare match
	TCNT0 = 0;					//se ini?ializeaz? num?r?torul cu 0
	OCR0 = 125;
	TIMSK |= 0b00000010;		//setarea bitilor iE pt intreruperile de la timer
}

void init_timer_2(){
	SREG = 1<<7;				//global interrupt enable
	TCCR2 = 0b00001100;			//in WGM01:0 am setat pe modul CTC clear time on compare match
	TCNT2 = 0;					//se ini?ializeaz? num?r?torul cu 0
	OCR2 = 125;
	TIMSK |= 0b10000000;
}

/*
void init_timer_1(){
	SREG = 1<<7;				
	TCCR1A = 0b10000000;			//in WGM01:0 am setat pe modul CTC clear time on compare match
	TCCR1B = 0b00001010;				
	TCNT1 = 0;			
	OCR1A = 1000;
	TIMSK |= 0b00010000;		
}
*/

float readADC (char ch){
	ADMUX  &= 0b11100000;	//Reseteaz? canalul de conversie
	ADMUX  |= ch;			//Seteaz? canalul conversiei
	ADCSRA |= (1<<6);		//Începe conversia
	
	while(ADCSRA & (1<<6));  //A?teapt? finalizarea conversiei
	adc_l = ADCL;
	adc_h = ADCH;
	return (float)((adc_h << 8) | adc_l);
}

void Init_adc(){
	ADMUX = 0b01000000;		//Referin?a - AVCC
	ADCSRA= 0b10000111;		//Activare ADC; Prescaler = 128;

}

void Init_INT0(){
	GICR  |= (1<<6);     // activarea întreruperii externe INT0
	MCUCR |= 0b00000010; // precizare c? evenimentul va avea loc pe frontul descresc?tor
	SREG  |= (1<<7);     // activarea globala a întreruperilor
}

void Init_INT1(){
	GICR  |= (1<<7);     // activarea întreruperii externe INT1
	MCUCR |= 0b00001100; // precizare c? evenimentul va avea loc pe frontul descresc?tor
	SREG  |= (1<<7);     // activarea globala a întreruperilor
}

void Init_INT2(){
	GICR  |= (1<<5);    
	MCUCSR &= ~(1 << 6); 
	SREG  |= (1<<7);     
}

ISR(INT0_vect){
	mode++;
	
}

ISR(INT1_vect){
	
	set++;
	if(set == 3)
	set = 0;
}

ISR(INT2_vect){
	if (set == 1){
		if(mode == 1){
			s++;
		}
		if(mode == 3){
			c_m++;
		}
		if(mode == 4){
			alarm_s++;
		}
		if(mode == 5){
			alarm1_m++;
		}
	}
	if(set == 2){
		if(mode == 1){
			m++;
		}
		if(mode == 3){
			h++;
		}
		if(mode == 4){
			alarm_m++;
		}
		if(mode == 5){
			alarm1_h++;
		}
	}
}
ISR(TIMER0_COMP_vect) {
	if (ms_adc == 999){
		adc = readADC(7);
		adc = adc/2 - 1;
		
	}else ms_adc++;
	
	if (s == alarm_s && m == alarm_m){
		stare = 0;
		ICR1H = TOP>>8;
		ICR1L = TOP;
		OCR1BH = ((TOP+1)/2)>>8;
		OCR1BL = (TOP+1)/2;
	}else OCR1B = 0;
	if ( c_m == alarm1_m && h == alarm1_h){
		stare = 0;
		ICR1H = TOP1>>8;
		ICR1L = TOP1;
		OCR1AH = ((TOP1+1)/2)>>8;
		OCR1AL = (TOP1+1)/2;
	}else OCR1A = 0;
}

ISR(TIMER2_COMP_vect){
	
	if(PINB & (1<<0)){	
		if(mode == 1){		//reset timer/alarm/clock
			ms = 0; s = 0; m = 0;
			OCR1B = 0;
		}		
		else if(mode == 3){
			c_ms = 0; c_s = 0; c_m = 0; h = 0;
		}
		else if(mode == 4){
			alarm_m = 0; alarm_s = 0;
		}
		else if(mode == 5){
			/*if(OCR1A == 0){
			alarm1_m = 0; alarm1_h = 0;
			}*/
			OCR1A = 0;
		}
	}
	
	if (PINB & (1<<1)){					//start/stop button
		if(stare == 0)
		stare = 1;
		else stare = 0;
	}
	
	if (stare == 1){
		if (ms == 999){
			s++;
			ms = 0;
			if(s == 60){
				s = 0;
				m++;
			}
		}else ms++;
	}
	
	
	
	if (c_ms == 999){				//clock
		c_s++;
		c_ms = 0;
		if(c_s == 60){
			c_s = 0;
			c_m++;
		}
		if(c_m == 60){
			c_m = 0;
			h++;
		}
		if(h == 24)
		h = 0;
		
	}else c_ms++;
	switch(mode){
		case 1: t = m; r = s; break;			//mod timer
		case 2: r = adc/100; t = adc; break;	//mod termometru
		case 3: t = h; r = c_m; break;			//mod ceas
		case 4: t = alarm_m; r =alarm_s; break;
		case 5: t = alarm1_h; r =alarm1_m; break;	//mod alarm
		case 6:  mode = 1;break;
	}
	
	if(set == 0){									//switch display by set button
		digit++;
		switch(digit){
			case 1: if(mode == 2){
				display(4, 10);
			}
			else display(4,r%10);
			break;
			
			case 2: if(mode == 2){
				display(3, 11);
			}
			else display(3,(r/10)%10);
			break;
			
			case 3: display(2,t%10);
			break;
			
			case 4: display(1,(t/10)%10); digit=0;
			break;
		}
	}
	else if(set == 1){

		digit1++;
		switch(digit1){
		
			case 1: if(mode == 2){
				display(4, 10);
			}
			else display(4,r%10); break;
			case 2: if(mode == 2){
				display(3, 11);
			}
			else display(3,(r/10)%10); digit1 = 0; break;
		}
		
	}
	else {
		digit2++;
		switch(digit2){
			case 1: display(2,t%10); break;
			case 2: display(1,(t/10)%10); digit2=0; break;
		}
	}
	
	
}

void main(){
	DDRA = 0b00001111;  
	DDRC = 0b11111111;
	DDRB = 0b00000000;
	DDRD |= (1<<4);
	DDRD |= (1<<5);

	//init_timer_1();
	init_Buzzer();
	init_timer(); init_timer_2(); Init_INT0(); Init_INT1();Init_adc(); Init_INT2(); 
	for(;;){
		
		 
	}
}