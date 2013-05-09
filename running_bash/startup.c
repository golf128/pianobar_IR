#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<unistd.h>
#include<time.h>
#include<lcd.h>
#include<wiringPi.h>
#include"../IRremote.c"
#include<string.h>

int main(void) {
    //printf("xx");
    wiringPiSetup();
    char key[100],tmpkey[1];
    int fvalue[100];
    int lcd1,value,temp=0,i;
    FILE *fd = NULL,*fd2 = NULL;
    fd = fopen("../config.txt","r");
    printf("what");
    fd2 = fopen("/home/pi/.config/pianobar/ctl","w");
    printf("file ready");
    //lcd1=lcdInit(2,16,8,11,10,14,13,12,3,2,0,7,9);
    //enableIRIn(4);
    //lcdPosition(lcd1,0,0);
    //lcdPuts(lcd1,"learning process");
    //lcdPosition(lcd1,0,1);
    //lcdPuts(lcd1,"press anykey");
    while(fscanf(fd, "%s",tmpkey) != EOF) {
        key[temp]=tmpkey;
        fscanf(fd,"%d",&fvalue[temp]);
        temp++;
    }
    enableIRIn(4);
    //while(1) {
      //  if(decode())
        //  {resume();if(get_result!=-1)  break;}
    //}
    //delay(7000);
    //while(fscanf(fd, "%s", key) != EOF) {
      //  int temp=1;
        //lcdPosition(lcd1,0,0);
        //lcdPrintf(lcd1,"keyyyyy");
        //lcdPosition(lcd1,0,1);
        //lcdPrintf(lcd1,"press for learn ");
        while(1) {
            //printf("wtf");
            if(decode()){
            for(i=0;i<temp;i++)
            {
                if(fvalue[i]==get_result())
                {  tmpkey[1]=key[i];
               fprintf(fd2,"%s",tmpkey[1]);// >> /home/pi/.config/pianobar/ctl; 
            }}

            //fprintf(fd2, "%d %s\n",get_result(),key);
            //lcdPosition(lcd1,0,0);
            //lcdPrintf(lcd1,"complete wait for next key");
            //delay(1500);
            resume();
            //temp=0;      }
          //else if(decode())resume();
        }}//return 0;
    }
    //lcdPosition(lcd1,0,0);
    //lcdPrintf(lcd1,"complete learning");
    //lcdPosition(lcd1,0,1);
    //lcdPrintf(lcd1,"                ");
    //return 0;

