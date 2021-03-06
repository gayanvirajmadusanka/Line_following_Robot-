  /*
 Motor Drive	Arduino		ATMEGA328P
 	INA      		D9			    PB1				PWM B
 	INB			    D13		  	  PB5				Dir B
 	INC 			  D12	   		  PB4				Dir A
 	IND 			  D10  		    PB2				PWM A

 	move(dir,Lspeed,Rspeed) ---->>> dir[1=forward, 0=backward]  if forward speed[50000 --> 5000]		if backward speed[5000 --> 50000]
 	turn(dir,speed) ---->>> dir[1=left, 0=right]

 Photo Diodes	Arduino		ATMEGA328P
 	    1 				D2	  		PD2
 	    2	  			D3		   	PD3
 	    3 		 		D4		   	PD4
 	    4 				D5		  	PD5
      
 UltraSonic    Arduino     ATMEGA328P
    Trig        A4          PC4
    Echo        A5          PC5        

 RGB LED 	Arduino		ATMEGA328P
 	  R 			 A1			PC1
 	  G			   A2			PC2
 	  B 			 A3 		PC3
 	  LDR 		 A6 		PC6

 */

#define F_CPU 16000000UL

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

#define BAUD 9600
#define MYUBRR ((F_CPU/(BAUD * 16UL))-1)

#define IR1 2
#define IR2 3
#define IR3 4
#define IR4 5

#define Blue 1
#define Green 2
#define Red 3
#define LDR 6

#define Echo 5
#define Trig 4

#define buzer 6

volatile int count;

float error = 0;
float maxError = 0;
float outError = 0;

float Kp = 1550;  //PID < 2500
float Kd = 190;//125;
float Ki = 0.8;//.5;

float P = 0;
float I = 0;
float D = 0;
int PIDvalue = 0;
float previousError = 0;
float previousError2 = 0;
float previousI = 0;

void init_IR(void);
float ReadLFSensor(void);
float speciaCases(void);
int calculatePID(void);

void init_PWM(void);
void turn(int dir,int speed);
void move(int dir,int Lspeed,int Rspeed, int pid);
void breakAll(void);

void init_DistSens(void);
void Trig_DistSens(void);

void init_buzer(void);

void detectColor(void);
void init_ColorSens(void);
void init_ADC(void);
uint16_t ReadADC(uint8_t ADCchannel);

void init_USART(void);
void USART_send(unsigned char data);
void USART_string(char* Stringptr);
void USART_number(uint32_t x);

double potval,rval,gval,bval;
char color;
int colorDelay = 0;

int main(void){

  init_IR();
  init_PWM();
  init_ADC(); 
  init_USART();
  init_ColorSens();
  init_DistSens();

  while(1){
    ReadLFSensor();
    speciaCases();
    calculatePID();
    breakAll();
    _delay_ms(50);
    Trig_DistSens();
    move(1,15000,15000,PIDvalue);

    }
  return 0;
  
}
void init_IR(void){
  DDRD = DDRD &~(1<<IR4)&~(1<<IR3)&~(1<<IR2)&~(1<<IR1);

}

float ReadLFSensor(void){
  
  if ( (PIND & (1<<IR1)) && (PIND & (1<<IR2)) && (PIND & (1<<IR3)) && (PIND & (1<<IR4)) ){ outError = 1;}
  else if ( (PIND & (1<<IR1)) && (PIND & (1<<IR2)) && (PIND & (1<<IR3)) ){ error = 3;}
  else if ( (PIND & (1<<IR1)) && (PIND & (1<<IR2)) && (PIND & (1<<IR4)) ){ error = 1;}
  else if ( (PIND & (1<<IR1)) && (PIND & (1<<IR3)) && (PIND & (1<<IR4)) ){ error = -1;}
  else if ( (PIND & (1<<IR2)) && (PIND & (1<<IR3)) && (PIND & (1<<IR4)) ){ error = -3;}
  else if ( (PIND & (1<<IR1)) && (PIND & (1<<IR2)) && (PIND & (1<<IR4)) ){ maxError = 3;}
  else if ( (PIND & (1<<IR1)) && (PIND & (1<<IR2)) ){ error = 2;}
  else if ( (PIND & (1<<IR1)) && (PIND & (1<<IR4)) ){ error = 0;}
  else if ( (PIND & (1<<IR3)) && (PIND & (1<<IR4)) ){ error =-2;}
  else if ( (PIND & (1<<IR1)) ){ error = 4;}
  else if ( (PIND & (1<<IR4)) ){ error = -4;}
  else if ( (PIND & (1<<IR2)) ){ maxError = 3;}
  else if ( (PIND & (1<<IR3)) ){ maxError = 4;}
  else { maxError = 2;}
    
  return error;
  return maxError;

}

float speciaCases(void){
  
  if( (previousError==0) && (error==4) ){error = 4;} //90
  else if( (previousError==4) && (outError==1) ){error = 4; outError = 0;} //90
  else if( (previousError==4) && (error==2) ){error=4;}               //90
  else if( (previousError==-4) && (error==4) ){error = -2;}           //junction
  else if( (previousError==4) && (maxError==2) ){error = -2;}         //junction
  else if( (previousError==-3) && (maxError==2) ){error = -2;}        //loop
  else if( (previousError==-2) && (maxError==3||maxError==4) ){error=-1;maxError=0;}//junction atfer loop
  else if( (previousError==2) && (maxError==2) && (outError==1) ){error = 4; maxError = 0; outError = 0;}      //color strips
  else if( (maxError==2) ){error = previousError; maxError = 0;}      //color strips

  return error;
  return maxError;
}

int calculatePID(void){
  
  previousError2 = previousError;
  P = error;
  I = I + error;
  D = error-previousError;
  PIDvalue = (Kp*P) + (Ki*I) + (Kd*D);
  previousError = error;

  return PIDvalue;
}

void init_PWM(void){
  DDRB |= 1<< PB5 | 1<<PB4 | 1<< PB2 | 1<<PB1;
  TCCR1A = 1<<COM1A1 | 1<<COM1B1 | 1<<WGM11 ;	
  TCCR1B = 0<<ICNC1 | 0<<ICES1 | 1<< WGM13 | 1<< WGM12| 0<<CS12 | 1<<CS11 | 0<<CS10; 
}

void move(int dir,int Lspeed, int Rspeed, int pid){
  
  ICR1 = 22000;
  OCR1A = Rspeed + pid;		//D9
  OCR1B = Lspeed - pid;		//D10
  
  if(dir == 1){
      PORTB = PORTB|(1<<4)|(1<<5);
    
  }else if(dir == 0){
      PORTB = PORTB &~(1<<4)&~(1<<5);
  }
}

void turn(int dir,int speed){
  
  ICR1 = 22000;
  OCR1A = speed;	
  OCR1B =speed;
  
  if(dir == 0){
    PORTB = PORTB|(1<<4); //pd2 >>> pb4		pd3 >>> pb5 
    PORTB = PORTB &~(1<<5);
  }else if(dir == 1){
    PORTB = PORTB| (1<<5);
    PORTB = PORTB &~(1<<4);
  }
}

void breakAll(void){
  PORTB =0xff;
}

void init_ColorSens(void){
  DDRC |= (1<<Red)|(1<<Green)|(1<<Blue);
  DDRC = DDRC &=~ (1<LDR);

  TCCR0A |= (1 << WGM01);
  OCR0A = 49; // Set the value that you want to count to
  TIMSK0 |= (1 << OCIE0A);    //Set the ISR COMPA vect
  sei();         //enable interrupts
  TCCR0B |= (1 << CS02);
}

ISR (TIMER0_COMPA_vect){  //action every 4ms
  colorDelay++;

  if(colorDelay==40){
    PORTC = PORTC|(1<<Red);
  }
  if (colorDelay==80){
    rval =(int)ReadADC(LDR);
    PORTC = PORTC &=~ (1<<Red);
  }
  if(colorDelay==120){
    PORTC = PORTC|(1<<Green);
  }
  if (colorDelay==160){
    gval =(int)ReadADC(LDR);
    PORTC = PORTC &=~ (1<<Green);
    colorDelay = 0;
    detectColor();
  }
}

void detectColor(void){
/*
    for (int i = 2; i < 4; i++){
      PORTC = PORTC|(1<<i);
      _delay_ms(40);
      potval =(int)ReadADC(LDR);
      if (i == 1){bval = potval;}
       else if (i == 2){gval = potval;}
      else if (i == 3){rval = potval;}
      PORTC = 0;
      _delay_ms(40);
    }
*/
    if((770>gval) && (700>rval)){ USART_string("Black\r\n");}   //Black
    //else if((rval>830) && (gval>840)){USART_string("White\r\n");} //White
    else if((gval>750) && ( (850>rval)&&(rval>740) )){USART_string("Green\r\n");} //Green
    else if(( (840>gval)&&(gval>700) ) && ( (920>rval)&&(rval>820) )){USART_string("Red\r\n");} //Red
    else{USART_string("Color Not Detected   ");/*USART_string("Rval = ");USART_number(rval);USART_string("  Gval = ");USART_number(gval);*/USART_string("\r\n");}

}

void Trig_DistSens(void){
  //_delay_ms(10);
  PORTC |= (1<<Trig);
  _delay_us(10);
  PORTC &= ~(1<<Trig);
}

void init_DistSens(void){
  DDRC |= (1<<Trig);
  DDRC &= ~(1<<Echo);     // Clear the PB0 pin
  // PB0 (PCINT0 pin) is now an input
  PORTC |= (1 << PORTC5);    // turn On the Pull-up
  // PB0 is now an input with pull-up enabled
  PCICR |= (1 << PCIE1);    // set PCIE0 to enable PCMSK0 scan
  PCMSK1 |= (1 << PCINT13);  // set PCINT0 to Trigger an interrupt on state change 
  sei();                    // turn on interrupts 
  TCCR2A |= (1 << WGM21); // Set the Timer Mode to CTC
  OCR2A = 11;  // Set the value that you want to count to
  TIMSK2 |= (1 << OCIE2A);    //Set the ISR COMPA vect
  sei();
  TCCR2B |= (1 << CS21);
  
}

ISR (TIMER2_COMPA_vect){  // timer0 overflow interrupt
  //event to be exicuted every 4ms here
  count++;
}

ISR (PCINT1_vect){
  if (bit_is_set(PINC,PC5)) {                 // Checks if Echo is high
    count = 0;                              // Reset Timer
  } else {
    uint16_t numuS = count;                 // Save Timer value
    //cli();                                // Disable Global interrupts
    //USART_string("Distance = ");
    USART_number((numuS*6)/58);     // Write Timer Value / 58 (cm). Distance in cm = (uS of Echo high) / 58
    USART_string("\n\r");   
  }    
}

void init_buzer(void){
  DDRD |= (1<<buzer);
  PORTD |= (1<<buzer);
  _delay_ms(10);
  PORTD &=~ (1<<buzer);
}

void init_ADC(void){
    // Select Vref=AVcc
    ADMUX |= (1<<REFS0);
    //set prescaller to 128 and enable ADC  
    ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);     
}

uint16_t ReadADC(uint8_t ADCchannel){
    //select ADC channel with safety mask
    ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F); 
    //single conversion mode
    ADCSRA |= (1<<ADSC);
    // maxError until ADC conversion is complete
    while( ADCSRA & (1<<ADSC) );
   return ADC;
}

void init_USART(void){
  UBRR0H = (uint8_t)(MYUBRR>>8);   
  UBRR0L = (uint8_t)(MYUBRR);   
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

unsigned char USART_receive(void){
  while(!(UCSR0A & (1<<RXC0)));
  return UDR0;
}

void USART_send(unsigned char data){
  while(!(UCSR0A & (1<<UDRE0)));
  UDR0 = data;
}

void USART_string(char* Stringptr){
  while(*Stringptr != 0x00){
    USART_send(*Stringptr);
    Stringptr++;
  }
}

void USART_number(uint32_t x){
  char value[10];
  int i = 0;

  do{
    value[i++] = (char)(x % 10) + '0';
    x /= 10;

  }while(x);

  while(i){
    USART_send(value[--i]);
  }
}