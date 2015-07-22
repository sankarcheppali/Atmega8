/*
Intelligent water heater using atmega8

User should be able to set the temperature to switch off the loads
(we can have two loads one load will switch on when the temperature is higher
than the given temperature, and other load will switch off)

---------Connections----------
Lcd connections:
PD0:RS
PD1:R/W
PD2:E
PD4-PD7:D4-D7(LCD)

Keyboard Connections:
PC0-save
PC2-10's place increment
PC3-1's place increment

Temperature sensor connection: we are using the thermistor
PC1-use water proof sensor (10k resistor from 5v
to PC1 and temistor form PC1 to gorund)
load connections:

PB1-load (active high, switches on when the teperature is higher than the given temperature)
PB2-load(active low,switches off when the teperature is higher than the given temperature)
PB3- Buzzer, will be active for 5 sec when tempearature corsses
the cut-off point
*/
#include<avr/io.h>
#include <avr/interrupt.h>
#include<stdlib.h>
#define F_CPU 16000000UL
#include<util/delay.h>
#include<avr/eeprom.h>
#include "lcd.c"  
#include "lcd.h" 
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 2
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000    
 
uint16_t samples[NUMSAMPLES];


uint16_t adc;
uint8_t temperature=0,minT=0,cmd,maxT;//minT location 2,maxT location 3
char s[5];

void initADC()
{
ADMUX=(1<<REFS0);  // For Aref=AVcc;
ADCSRA=(1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1);//|(1<<ADPS0); //Prescalar div factor =128
}

int ReadADC(uint8_t ch)
{
   //Select ADC Channel ch must be 0-7
   //clear the channel first
   ch=ch&0b00000111;
   ADMUX|=ch;

   //Start Single conversion
   ADCSRA |= (1<<ADSC);

   //Wait for conversion to complete
   while(!(ADCSRA&(1<<ADIF)));

   //Clear ADIF by writing one to it
   //Note you may be wondering why we have write one to clear it
   //This is standard way of clearing bits in io as said in datasheets.
   //The code writes '1' but it result in setting bit to '0' !!!

   ADCSRA|=(1<<ADIF);

   return(ADC);
}
 float getTemperature()
 {
   
  uint8_t i;
  float average;
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = ReadADC(1);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
 
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
 
   return steinhart;
 }

  void checkLoad()  //to set the loads acordin to the temperature
  {
    if(temperature>=maxT)
	   {
	     PORTB|=(1<<PB1);
		   PORTB&=~(1<<PB2);
       PORTB|=(1<<PB3);
       _delay_ms(5000);
       PORTB&=~(1<<PB3);
	   }
     else if(temperature<minT)
     {

       PORTB|=(1<<PB2);
       PORTB&=~(1<<PB1);
       PORTB|=(1<<PB3);
       _delay_ms(5000);
       PORTB&=~(1<<PB3);

     }
  }

  void updateLCD()  //update the lcd with current temperature and target temperature
  { 
     checkLoad();
     lcd_init(LCD_DISP_OFF);
	 lcd_init(LCD_DISP_ON);
	 lcd_clrscr();
	 lcd_puts("minT:");
	 itoa(minT,s,10);
	 if(minT<10)
	 lcd_puts("0");
	 lcd_puts(s);

   lcd_puts(",maxT:");
   itoa(maxT,s,10);
   if(maxT<10)
   lcd_puts("0");
   lcd_puts(s);
   
	 lcd_gotoxy(0,1);
	 lcd_puts("PRESENT TEMP:");
	 if(temperature<10)
	 lcd_puts("0");
	 itoa(temperature,s,10);
	 lcd_puts(s);
  }

  
//blocking method ,scans key board and gives the grounded key number (ex: for pc2 it will give 32 3 for c and 2 for pin number,0 if no key pressed)
uint8_t readKeyboard()
 {
   while( (PINC&(0x0D))!=0x0D ); //wait until all the keys are released
   while( (PINC&(0x0D))==0x0D ); //wait until some key is pressed
  _delay_ms(100);
   if((PINC&(1<<PC0))==0 )
   {
       return 32;
   }
   else if((PINC&(1<<PC2))==0 )
   {
       return 33;
   }
   else if((PINC&(1<<PC3))==0 )
   {
       return 34;
   }
  else 
  {
    return 0;
  }

 }
 void updateTargetOnLCD(int target,char[] s)
{
   lcd_init(LCD_DISP_OFF);
   lcd_init(LCD_DISP_ON);
   lcd_clrscr();
   lcd_puts(s);
   itoa(target,s,10);
   if(target<10)
   lcd_puts("0");
   lcd_puts(s);
}
uint8_t update(uint8_t target,char *s,uint16_t location)
{
  uint8_t hDigit=target/10,lDigit=target%10;
  while(1){
      sei();      
     cmd=readKeyboard();
       cli();
     if(cmd==32)
     {
         hDigit++;
       if(hDigit>=10)
       {
         hDigit=0;
       }
       target=hDigit*10+lDigit;
     }
     else if(cmd==33)
     {  
     lcd_clrscr();
     lcd_puts("saving the data..");  
     _delay_ms(100);
       target=hDigit*10+lDigit;
        while (!eeprom_is_ready());
                      eeprom_write_byte((uint16_t *)(location),(uint8_t)(target));//save temperaure data to the EEPROM
           sei();           
        return target;  
      }
    else if(cmd==34)
     {
           lDigit++;
           if(lDigit>=10)
       lDigit=0;
       target=hDigit*10+lDigit;
     }
   }
}
  int main()
  {
    DDRC=0x00;  //KEY BOARD
    PORTC=0xFD;  //ACTIVATE PULL UP(PC1 don't need any pullup)

    DDRB=0x0E;  //TIMER ENABLE KEYS
    PORTB=0x01; //ACTIVATE PULL UPS
    PORTB&=~(1<<PB1);//PB1 SHOULD BE OFF
    PORTB|=(1<<PB2);//PB2 SHOULD BE ON
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    lcd_puts("....BOOTING....");
               //setup ports
     initADC(); //init ADC
	            
	//init timer for 1 sec interrupt for periodic update of the temperature on the LCD
      TCCR1B=(1<<WGM12)|(1<<CS12);
                      OCR1A=62500;
               TIMSK|=(1<<OCIE1A);
         while (!eeprom_is_ready());
         minT=eeprom_read_byte((const uint16_t *)(2));  //read the saved power from EEROM
       if(minT>100)
	   {
	     minT=99;
	   }

         while (!eeprom_is_ready());
         maxT=eeprom_read_byte((const uint16_t *)(3));  //read the saved power from EEROM
         if(maxT>100)
         {
            maxT=99;
         }
     
	 _delay_ms(1000);
	 updateLCD();
	 sei();
   uint8_t toggle=0;
   while(1)
   { 
       cmd=readKeyboard();
        if(cmd==33)
        {
          if(toggle==0){
            toggle=1;
            minT=update(minT,"minT:",2);
           }
         else{
             toggle=0;
             maxT=update(maxT,"maxT:",3);
           }
        }
       
   }
 }

ISR (TIMER1_COMPA_vect)
 {
   cli();
     temperature=getTemperature();
	   updateLCD();
   sei();
 }
