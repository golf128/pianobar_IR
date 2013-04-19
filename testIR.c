#include<stdio.h>
#include<wiringPi.h>
#include"IRremote.c"

int main(void) {
wiringPiSetup();
pinMode(5,OUTPUT);
enableIRIn(4);
for(;;)
{
 if(decode())
  digitalWrite(5,1);
  delay(500);
 digitalWrite(5,0);
resume();
}
}
