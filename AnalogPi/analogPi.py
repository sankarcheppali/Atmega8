#AnalogPi is AddOn borad to Raspberry Pi, Using this board you read anlog vlues
#Features:
#6- Analog Pins(Max Input voltage -5)
#11 Digital IO(5v)
#BaudRate:9600bps,8-bit,no parity checking,1-stop
#each command consits of 4 bytes,
#First byte is strat byte(0x0D)
#second byte is pin information, 5th bit(0x20) should be 1
#third byte represents teh  requested operation
# for analog read cmd byte  is 98(6th,5th should be 1 and 7th bit should be 0)
# for digital read cmd byte is 0x04 (For all digital operations 2nd bit should be 1 and bits left to that should be 0)
#for digital write cmd byte is 0x07/0x05(1st bit represents the write 1 or write 0 operation, 0x07 for write 1 and 0x05 is write 0)
#fourth byte represents the end of pocket

import serial
import time
class AnalogPi:
    ser=None
    def __init__(self,com):
        self.ser=serial.Serial()
        self.ser.baudrate=9600
        self.ser.timeout=2
        self.ser.port=com
    def analogRead(self,pin):
        if((pin>=0) and (pin<=5)): # anlog pins are form 0-5(A0-A5)
            self.ser.open()
            sendRequest(self,pin,98)# first byte is pin, second byte is operation command
            if(waitForResponce(self,4,1000)==1):
                responce=self.ser.read(4)
                temp= (responce[1]<<8)|(responce[2])
            else:
                temp=None
            self.ser.close()
            return temp

    def digitalRead(self,pin):
        if(pin>=0 and pin<=11 ): #digital pins are from 0 to 11
            self.ser.open()
            sendRequest(self,pin,4)
            if(waitForResponce(self,4,1000)==1):
                responce=self.ser.read(4)
                temp=responce[2]&0xFD
            else:
                temp=None
            self.ser.close()
            return temp

    def digitalWrite(self,pin,bit):
        if(pin>=0 and pin<=11 ): #digital pins are from 0 to 11
            self.ser.open()
            if(bit!=0):
                bit=1
            sendRequest(self,pin,(5|(bit<<1)))

            if(waitForResponce(self,4,1000)==1):
                responce=self.ser.read(4)# responce if four byte lenght
                #print responce
                temp=ord(responce[2])&0xFD
            else:
                temp=None
            self.ser.close()
            return temp

def sendRequest(self,pinData,cmdData):
      #str=[0x0D,(pinData|0x20),cmdData,0x0A]
      str=chr(0x0D)+chr((pinData|0x20))+chr(cmdData)+chr(0x0A)
      self.ser.write(str)

def waitForResponce(self,lenght,timeOut): # it will wait for the specified number of bytes, timeOut in ms
     counter=0
     while(self.ser.inWaiting<lenght and counter<timeOut):
         time.sleep(0.001)
         counter=counter+1
     if(self.ser.inWaiting>=lenght):
         return 1
     else:
         return 0
