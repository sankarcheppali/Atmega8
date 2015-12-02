
#include "analog.h"

void initADC()
{
   ADMUX=(1<<REFS0);  // For Aref=AVcc;
   ADCSRA=(1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1);//|(1<<ADPS0); //Prescalar div factor =128
}

uint16_t readADC(uint8_t ch)
{
  ch=ch&0b00000111;
                  
   //ADMUX&=0xF8;;
    
   ADMUX=ch|( ADMUX&0xF8);
        
//    adc_cf=1;
   //Start Single conversion
   ADCSRA |= (1<<ADSC);

        

   //Wait for conversion to complete
   while(!(ADCSRA&(1<<ADIF)));
   //Clear ADIF by writin g one to it
   //Note you may be wondering why we have write one to clear it
   //This is standard way of clearing bits in io as said in datasheets.
   //The code writes '1' but it result in setting bit to '0' !!!
   ADCSRA|=(1<<ADIF);        
  //adc_cf=0;
   return(ADC);
}
