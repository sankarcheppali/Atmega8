/*
Analog Pi, Model No:5650

This is add on board to raspberry Pi
This board can read analog values and give it to the
raspberry pi.

Features:
6- Analog Pins
12- Digital pins(with internal pull ups in read mode)
<0x0D>2 byte cmd<0x0A>
*/

 #define F_CPU 16000000ul
 #include<avr/interrupt.h>
 #include"analogpi.h"
 #include"analog.c"
 #include"uart.c"

 uint8_t buffer[15];
 uint8_t Receivedbyte,k;

 void digitalReadOut(uint8_t pin)
 {
    uint8_t localPin;
    if(pin<6)  //PORTB
	{
	   digitalResponse(PORTB&(1<<pin));
	}
	else if(pin<12)
	{
	   localPin=pin-4;
	   digitalResponse(PORTD&(1<<localPin));
	}
	else if(pin<18)
	{
	  localPin=pin-12;
	  digitalResponse(PORTC&(1<<localPin));
	}
	else
	{
	  errorResponse();
	}
 }

 void digitalRead(uint8_t pin)
 {
   //first make pin as input, then give pull up, then read
	  uint8_t localPin;
    if(pin<6)  //PORTB
	{
	   DDRB&=~(1<<pin);
	   PORTB|=(1<<pin);
	   digitalResponse(PINB&(1<<pin));
	}
	else if(pin<12)
	{
	   localPin=pin-4;
       DDRD&=(1<<localPin);
       PORTD|=(1<<localPin);
	   digitalResponse(PIND&(1<<localPin));
	}
	else if(pin<18)
	{
	  localPin=pin-12;
      DDRC&=(1<<localPin);
      PORTC|=(1<<localPin);
	  digitalResponse(PINC&(1<<localPin));
	}
	else
	{
	  errorResponse();
	}
 }

 void digitalWrite(uint8_t pin,uint8_t bit)
 {
   uint8_t localPin;
    if(pin<6)  //PORTB
    {
	   DDRB|=(1<<pin);
	   if(bit)
	     PORTB|=(1<<pin);
	   else
	     PORTB&=~(1<<pin);
	   digitalResponse(PORTB&(1<<pin));
	}
	else if(pin<12)
	{
	    localPin=pin-4;
	    if(bit)
	     PORTD|=(1<<localPin);
	    else
	     PORTD&=~(1<<localPin);	
	   digitalResponse(PORTD&(1<<localPin));
	}
	else if(pin<18)
	{
	   localPin=pin-12;
	    if(bit)
	     PORTC|=(1<<localPin);
	    else
	     PORTC&=~(1<<localPin);	
	  digitalResponse(PORTC&(1<<localPin));
	}
	else
	{
	  errorResponse();
	}  
 }
 void action()
 {
    //cmd byte will be in 1st and 2nd postions
   //check for analog read
   if((buffer[2]&(0xE0))==0x60)
   {
     //analog read
	 analogResponse(readADC(buffer[1]&0x0F));
	
   } 
   else if( (buffer[2]&(0xFC))==0x04 )//check for digital action
   {
     //check for read or write
	 if((buffer[2]&(0x03))==0x00)
	 {
	    //digital read
		digitalRead(buffer[1]&(0x1F));
	 }
	 else if((buffer[2]&(0x03))==0x02)
	 {
	   //digital read output register
	   digitalReadOut(buffer[1]&(0x1F));
	 }
	 else
	 {
	   //digital write
	   digitalWrite(buffer[1]&(0x1F),buffer[2]&(0x02));
	 }
   }
   else
   { 
     sendResponse("OK");
   }
 }

int main()
{
  initADC();
  initUART();
  DDRC|=(1<<PC5);
  while(1)
  {
  
  }
}
/**************************************/
ISR(USART_RXC_vect)
   {
      Receivedbyte = UDR;
      PORTC|=(1<<PC5);
      /*  if((k==0)&&(Receivedbyte==0x0D))
          {
             cmd_prog=1;
          }

        if((k!=1)&&(Receivedbyte==0x0A))
          {
             cmd_prog=0;
             cmd_com=1;
          } */

        if(k<20)
          {
            if(((k==0)&&(Receivedbyte==0x0D))||(k))
            {  
			   buffer[k]=Receivedbyte;
               k++;
			   if(Receivedbyte==0x0A)
			   {
			      //disable interrupts, analyze the cmd
				  cli();
				  action();
				  k=0;
				  sei();//enable interrupts
			   } 
			}
          }
		  else
		  {
		    k=0;
		  }
		  PORTC&=~(1<<PC5);
       }

/**************************************/

void sendResponse(uint8_t *data)
{
  uartWriteByte(0x0D);
  uartWriteString(data);
  uartWriteByte(0x0A);
}

void digitalResponse(uint8_t response)
{
  uint8_t data[10];
  data[0]=buffer[1];
  if(response)
   data[1]=buffer[2]|0x02;
  else
   data[1]=buffer[2]&0xFD;
  data[2]=0x00;
  sendResponse(data);
}

void errorResponse()
{
   uint8_t data[10];
   data[0]=0xFF;
   data[1]=0xFF;
   data[2]=0x00;
  sendResponse(data);
}

void analogResponse(uint16_t analog)
{
  uint8_t data[10];
  data[0]=(analog>>8)&0xFF;//lsb first
  data[1]=(analog)&0xFF;
  data[2]=0x00;
 sendResponse(data);
}
