#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<unistd.h>
#include<time.h>
#include<lcd.h>
#include<wiringPi.h>
#include"IRremote.c"

int main(void) {
printf("Hello world  ");
wiringPiSetup();
pinMode(5,OUTPUT);
int lcd1;
lcd1=lcdInit(2,16,8,11,10,14,13,12,3,2,0,7,9);
enableIRIn(4);
//printf(" %d  %d ",get_result(),get_type());
lcdPosition(lcd1,0,0);
lcdPuts(lcd1,"test from Rpi");
lcdPosition(lcd1,0,1);
lcdPuts(lcd1,"Hi daveallenvw");
for(;;)
{//digitalWrite(5,0);
 if(decode())
 {printf( "value= %d type= %d",get_result(),get_type());// digitalWrite(5,1);
  lcdPosition(lcd1,0,0);
    lcdPrintf(lcd1,"value = %d ",get_result());
lcdPosition(lcd1,0,1);
lcdPrintf(lcd1,"type = %d       ",get_type());

  resume();
}
}
}
