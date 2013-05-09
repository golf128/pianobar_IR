#include <stdio.h>
#include <wiringPi.h>
int main (void)
{
printf ("Raspberry Pi blink\n") ;
if (wiringPiSetup () == -1)
return 1 ;
pinMode (5, OUTPUT) ;
for (;;)
{digitalWrite (5, 1) ; // On
delay (500) ; // mS
digitalWrite (5, 0) ; // Off
delay (500) ;
}
return 0 ;
}
