#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<unistd.h>
#include<time.h>
#include<lcd.h>
#include<wiringPi.h>
#include"/home/pi/pianobar_IR/IRremote.c"

int main(void) {
wiringPiSetup();
//delay(4000);
int lcd1;
char temp[160];
FILE *fd = NULL;
fd = fopen("/home/pi/pianobar_IR/running_bash/out","r");
lcd1=lcdInit(2,16,8,11,10,14,13,12,3,2,0,7,9);
lcdPosition(lcd1,0,0);
fgets(temp,100,fd);
temp[strlen(temp)-1]= NULL;
lcdPrintf(lcd1,"%s",temp);
lcdPosition(lcd1,0,1);
fgets(temp,100,fd);
temp[strlen(temp)-1]= NULL;
lcdPrintf(lcd1,"%s",temp);
}
