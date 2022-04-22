#include <SPI.h>
#include <DMD.h>
#include <TimerOne.h>
#include <Wire.h> 
#include <virtuabotixRTC.h> 
#include "SystemFont5x7.h"
#include "Arial_black_16.h"

DMD dmd(2, 1);
virtuabotixRTC myRTC(3, 4, 5);

bool dataStat = false;
bool tsunami = false;
bool tsunami_change = false;

int timerDate = 0;

String FirstDataIncome;
String SecondDataIncome;
String ThirdDataIncome;
String FourthDataIncome;
String LastDataIncome;
String SerialIncome;

void setup()
{
  Serial.begin(19200);
  Timer1.initialize(5000);         
  Timer1.attachInterrupt(ScanDMD); 
  dmd.clearScreen(true);
  
  //myRTC.setDS1302Time(00, 50, 13, 6, 5, 8, 2019);
}

void loop()
{
  String scroll_left;
  if(dataStat)
  {
    dmd.selectFont(Arial_Black_16);
    if(!tsunami) scroll_left = "Info Gempa Mag " + SecondDataIncome + "SR Depth " + ThirdDataIncome + "Km Jarak " + FirstDataIncome + "Km";
    else scroll_left = "Berpotensi Tsunami, Segera Evakuasi";
  }
  else
  {
    dmd.selectFont(SystemFont5x7);
    scroll_left = "- Automatic Tsunami Early Warning System -";
  }
  
  int panjang_teks = scroll_left.length() + 1;
  char teks[panjang_teks];
  int panjang_runningteks = 32;
  scroll_left.toCharArray(teks,panjang_teks);
  int b = 64;
  for(int a=0;a<panjang_teks;a++) {
    panjang_runningteks = panjang_runningteks + dmd.charWidth(teks[a]);
  }
  
  while(b != -panjang_runningteks) {
    dmd.drawString(b,0,teks,panjang_teks,GRAPHICS_NORMAL);
    delay(50);
    b--;
    SerialMonitor();

    if(!dataStat){
      String TimeDisplayRaw;
      myRTC.updateTime();
      timerDate++;
      if(timerDate < 200){
        String jam;
        jam = myRTC.hours;
        if(jam.toInt() < 10) jam = "0" + String(myRTC.hours);
        String menit;
        menit = myRTC.minutes;
        if(menit.toInt() < 10) menit = "0" + String(myRTC.minutes);
        String detik;
        detik = myRTC.seconds;
        if(detik.toInt() < 10) detik = "0" + String(myRTC.seconds);
        TimeDisplayRaw = String(myRTC.hours) + ":" + String(myRTC.minutes) + ":" + String(myRTC.seconds) + "   ";
      }else{
        TimeDisplayRaw = String(myRTC.dayofmonth) + "/" + String(myRTC.month) + "/" + String(myRTC.year) + " ";
      }
      char TimeDisplay[12];
      if(timerDate > 400) timerDate = 0;
      TimeDisplayRaw.toCharArray(TimeDisplay,12);
      dmd.drawString(0,8,TimeDisplay,12, GRAPHICS_NORMAL);
    } 
    if(tsunami_change){
        b = -panjang_runningteks;
        tsunami_change = false;
        dmd.clearScreen( true );
    }
  }
}

void ScanDMD(){dmd.scanDisplayBySPI();}

void SerialMonitor(){
  if(Serial.available() > 0){    
    //Data Format (100!5&10$T Latitude $ Longtitude % Magnitude & Kedalaman
    SerialIncome = Serial.readString();
    int inFirstMessage;
    inFirstMessage = SerialIncome.indexOf('(');
    
    if(inFirstMessage != -1){
      int inSecondMessage = SerialIncome.indexOf('!');
      int inThirdMessage = SerialIncome.indexOf('&');
      int inLastMessage = SerialIncome.indexOf('$');
      int cek = SerialIncome.lastIndexOf('S');
      
      FirstDataIncome = SerialIncome.substring((inFirstMessage + 1), inSecondMessage);
      SecondDataIncome = SerialIncome.substring((inSecondMessage + 1), inThirdMessage);
      ThirdDataIncome = SerialIncome.substring((inThirdMessage + 1), inLastMessage);

      if(cek != -1) tsunami = true;
      else tsunami = false;
      
      dataStat = true;           
      tsunami_change = true;
    }
    else if(SerialIncome.indexOf('A') != -1)
    {
      Serial.print("OK");
      dataStat = false;
      tsunami = false;
      tsunami_change = true;
    }
    else
    {
      dataStat = false;
      tsunami_change = true;
      tsunami = false;
    }
  } 
}
