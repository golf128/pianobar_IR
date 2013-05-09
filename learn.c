#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<unistd.h>
#include<time.h>
#include<lcd.h>
#include<wiringPi.h>
#include"IRremote.c"
#include<string.h>

int main(void) {
    wiringPiSetup();
    char key[1];
    int lcd1,value;
    FILE *fd = NULL,*fd2 = NULL;
    fd = fopen("input.txt","r");
    fd2 = fopen("config.txt","w");
    lcd1=lcdInit(2,16,8,11,10,14,13,12,3,2,0,7,9);
    lcdPosition(lcd1,0,0);
    lcdPuts(lcd1,"learning process");
    lcdPosition(lcd1,0,1);
    lcdPuts(lcd1,"press anykey");
     enableIRIn(4);
    while(1) {
        if(decode())
          {resume();if(get_result()!=-1)  break;}
    }
    delay(3000);
    while(fscanf(fd, "%s", key) != EOF) {
        int temp=1;
        lcdPosition(lcd1,0,0);
        lcdPrintf(lcd1,"key = %s         ",key);
        lcdPosition(lcd1,0,1);
        lcdPrintf(lcd1,"press for learn ");
        while(temp) {
            if(decode()&&get_result()!=-1){
            fprintf(fd2, "%s\n%d\n",key,get_result());
            lcdPosition(lcd1,0,0);
            lcdPrintf(lcd1,"complete wait for next key");
            delay(1500);
            resume();
            temp=0;      }
          else if(decode())resume();
        }
    }
    lcdPosition(lcd1,0,0);
    lcdPrintf(lcd1,"complete learning");
    lcdPosition(lcd1,0,1);
    lcdPrintf(lcd1,"                ");
    return 0;
}
