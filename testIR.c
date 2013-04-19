#include<stdio.h>
#include<stdlib.h>
#include<wiringPi.h>
#include"IRremote.c"

int main(void) {
printf("Hello world  ");
wiringPiSetup();
pinMode(5,OUTPUT);
enableIRIn(4);
printf(" %d  %d ",get_result(),get_type());
for(;;)
{digitalWrite(5,0);
 if(decode())
 {printf( "value= %d type= %d",get_result(),get_type());// digitalWrite(5,1);
  return 0;
   //delay(100);
 //digitalWrite(5,0);
//resume();
}
}
}
