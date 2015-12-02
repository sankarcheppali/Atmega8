//#include "uart.h"
//void uartinit(uint16_t ubrr_value)
void initUART()
{
        
    UCSRB = 0x98;
    UCSRC = 0x06;
    UBRRL = 103;//9600 baud rate,Analog Pi Uses 16MHZ crystal
    UBRRH = 0x00;
    sei();
}

   
char uartReadByte()
{
   while(!(UCSRA&(1<<RXC)));
   return UDR;
}

void uartWriteByte(uint8_t data)
 {
    while(!(UCSRA&(1<<UDRE)));
    UDR=data;
 }
 void uartWriteString(uint8_t *str)
 {
      while(*str)
      {
         uartWriteByte(*str);
         str++;
      }                    
 }


