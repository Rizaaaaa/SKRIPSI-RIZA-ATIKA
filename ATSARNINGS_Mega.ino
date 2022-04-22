#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DFPlayer_Mini_Mp3.h>

LiquidCrystal_I2C lcd(0x27,20,4);

const int InMenu = 4;
const int InUp = 8;
const int InDown = 6;
const int InBack = 10;
const int OutRelay = 2;

bool MenuStat = false;
bool UpStat = false;
bool DownStat = false;
bool BackStat = false;
bool change = true;
bool relay = false;
bool holdMenu = false;
bool dataStat = false;
bool SerialTest = false;
bool tsunami = false;
bool tsunami_change = false;
bool dotTest = false;

int MenuPost = 0;
int a = 0;
int timerMenu = -1;
int timerDate = 0;
int setVolMenu = 25;
int countWarning = 0;

String FirstDataIncome;
String SecondDataIncome;
String ThirdDataIncome;
String FourthDataIncome;
String LastDataIncome;
String SerialIncome;

void setup()
{
  Serial1.begin(19200); //GSM
  Serial2.begin(9600); //DFPlayer
  Serial3.begin(19200); //Nano
  mp3_set_serial(Serial2);
  mp3_set_volume(25);
  
  lcd.init();
  lcd.backlight();
  
  pinMode(InMenu, INPUT);
  pinMode(InUp, INPUT);
  pinMode(InDown, INPUT);
  pinMode(InBack, INPUT);
  pinMode(OutRelay, OUTPUT);
}

void loop()
{
  if(dataStat){
    countWarning++;
    timerDate++;
    if(countWarning ==  1) Warning();
    if(countWarning == 200) Warning();
    if(countWarning == 400) countWarning = 0;
    if(timerDate == 900 && tsunami) mp3_play(1);
    if(timerDate == 500 && !tsunami) mp3_play(3);
  }else{
    SerialMonitor();
    if(timerMenu > 0 && !holdMenu){
      timerMenu--;
      change = true;
    }else{
      MainMenu();
    }
  }
  
  if(digitalRead(InMenu) == HIGH) MainMenu();
  if(digitalRead(InUp) == HIGH) MainMenu();
  if(digitalRead(InDown) == HIGH) MainMenu();
  if(digitalRead(InBack) == HIGH) MainMenu();
  
  if(MenuPost == 200) volSirene();

  delay(10);
}

void SerialMonitor(){
  if(SerialTest){
    lcd.setCursor(0,0);
    lcd.print("Memulai Test Server");
    lcd.setCursor(0,1);
    lcd.print("> Mohon Tunggu");
    Serial1.println("AT");
    delay(100);
  }
  if(Serial1.available() > 0){    
    //Data Format (100!5&10$T Latitude $ Longtitude % Magnitude & Kedalaman
    SerialIncome = Serial1.readString();
    int inFirstMessage;

    if(SerialTest){
      inFirstMessage = SerialIncome.indexOf('O');
      lcd.setCursor(0,2);
      lcd.print("GSM Ready");
    }else{
      inFirstMessage = SerialIncome.indexOf('(');
    }
      
    if(inFirstMessage != -1){
      if(SerialTest){
        FirstDataIncome = SerialIncome.substring(inFirstMessage, (inFirstMessage + 2));
        lcd.setCursor(0,3);
        lcd.print(FirstDataIncome);
        //lcd.print(SerialIncome);
      }
      else
      {
        int inSecondMessage = SerialIncome.indexOf('!');
        int inThirdMessage = SerialIncome.indexOf('&');
        int inLastMessage = SerialIncome.indexOf('$');
        int cek = SerialIncome.lastIndexOf('S');
        
        FirstDataIncome = SerialIncome.substring((inFirstMessage + 1), inSecondMessage);
        SecondDataIncome = SerialIncome.substring((inSecondMessage + 1), inThirdMessage);
        ThirdDataIncome = SerialIncome.substring((inThirdMessage + 1), inLastMessage);
       
        if(cek == -1) {
          tsunami = false;
          mp3_play(3);
          timerDate = 0;
        }else{
          tsunami = true;
          mp3_play(2);
          timerDate = 0;
        }
        
        dataStat = true;           
        timerMenu = 0;
        a = 0;
        MenuPost = 0;
        lcd.clear();
        Warning();
        tsunami_change = true;
        
        Serial3.print(SerialIncome.substring(inFirstMessage));
      }
    }
  } 
  if(Serial3.available() > 0){
    SerialIncome = Serial3.readString();
    lcd.setCursor(0,2);
    lcd.print("DotMatriks Ready");
    lcd.setCursor(0,3);
    lcd.print(SerialIncome);
  }
}

void Warning(){
  timerMenu++;
  if(timerMenu == 1){
    lcd.setCursor(0,0);
    lcd.print("     WARNING!!!    ");
    if(tsunami){
      lcd.setCursor(0,1);
      lcd.print(" BERPOTENSI TSUNAMI");
      lcd.setCursor(0,2);
      lcd.print("  SEGERA EVAKUASI");
    }else{
      lcd.setCursor(0,1);
      lcd.print("  TIDAK BERPOTENSI");
      lcd.setCursor(0,2);
      lcd.print("     TSUNAMI");
      lcd.setCursor(0,3);
      lcd.print("    TETAP TENANG");
    }
  }else{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("BAHAYA GEMPA!");
    lcd.setCursor(0,1);
    lcd.print("MAG : ");
    if(FirstDataIncome.length() < 10) lcd.print(SecondDataIncome);
    lcd.print(" SR");
    lcd.setCursor(0,2);
    lcd.print("DEPTH : ");
    if(SecondDataIncome.length() < 10) lcd.print(ThirdDataIncome);
    lcd.print(" KM");
    lcd.setCursor(0,3);
    lcd.print("Jarak : ");
    if(ThirdDataIncome.length() < 10) lcd.print(FirstDataIncome);
    lcd.print(" KM");
  }
  if(timerMenu > 2) {
    timerMenu = 0;
    lcd.clear();
  }
}
/*
void setJam(){
  lcd.setCursor(0,0);
  lcd.print("Set Waktu");
  lcd.setCursor(0,1);
  lcd.print("JJ/MM/DD");
  lcd.setCursor(0,2);
  lcd.print(myRTC.hours);
  lcd.print(":");
  lcd.print(myRTC.minutes);
  lcd.print(":");
  lcd.print(myRTC.seconds);
  lcd.setCursor(1,3);
  lcd.print("^");
  
  //myRTC.setDS1302Time(00, 59, 23, 6, 10, 1, 2014);
}

void setTanggal(){
  lcd.setCursor(0,0);
  lcd.print("Set Hari");
  lcd.setCursor(0,1);
  lcd.print("DD/MM/YY");
  lcd.setCursor(0,2);
  lcd.print(myRTC.dayofmonth);
  lcd.print(":");
  lcd.print(myRTC.month);
  lcd.print(":");
  lcd.print(myRTC.year);
  lcd.setCursor(1,3);
  lcd.print("^");
}*/

void testSirene(){
  lcd.setCursor(0,0);
  lcd.print("Memulai Test Sirene");
  lcd.setCursor(0,1);
  lcd.print("Sirene Diaktifkan");
  mp3_play(1);
}

void volSirene(){
  lcd.setCursor(0,0);
  lcd.print("Set Volume Sirene");
  lcd.setCursor(0,1);
  lcd.print("Tekan Up/Down");
  lcd.setCursor(0,2);
  lcd.print("Vol : ");
  lcd.print(String(setVolMenu*4) + " %");
  if(DownStat || UpStat) mp3_set_volume(setVolMenu);;
}

void MainMenu(){
  if(timerMenu == 0 && change){
    a = 0;
    MenuPost = 0;
    lcd.clear();
  }
  if(a != 0 && change){
    menu();
    change = false;
  }else if(change){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("  Automatic Tsunami");
    lcd.setCursor(0,1);
    lcd.print("Early Warning System");
    lcd.setCursor(0,2);
    lcd.print("Tersinkronisasi BMKG");
    lcd.setCursor(0,3);
    lcd.print("  dan Tempat Ibadah");
    change = false;
  }

  if(digitalRead(InMenu) == HIGH){
    if(dataStat){
      dataStat = false;
      tsunami = false;
      change = true;
      tsunami_change = true;
      Serial3.print("a");
      mp3_stop();
    }else{
      if(!MenuStat){
        change = true;
        timerMenu = 250;
        MenuStat = true;
        lcd.clear();
        if(a==0){
          a = 1;
        }
        else if(a==1){
          lcd.clear();
          switch(MenuPost){
            case 0:
              MenuPost = 10;
              holdMenu = true;
            break;
            case 1:
              MenuPost = 20;
              holdMenu = true;
            break;
            case 2:
              MenuPost = 30;
              holdMenu = true;
            break;
          }
          a = 2;
        }
        else if(a == 2 && MenuPost != 10){
          lcd.clear();
          switch(MenuPost){
            case 20:
              MenuPost = 200;
              holdMenu = true;         
            break;
            //case 30:
            //  MenuPost = 300;
            //  holdMenu = true;
            //break;
          }
          a = 3;
        }
      }
    }
  }else{
    MenuStat = false;
  }
  
  if(digitalRead(InUp) == HIGH && (a == 1 || MenuPost > 100)){
    if(UpStat == false){
      change = true;
      UpStat = true;
      timerMenu = 250;
      //a=1;
      if(MenuPost < 5){
        MenuPost++;
        if(MenuPost > 2) MenuPost = 0;
      }else if(MenuPost == 200 && setVolMenu > 0){
        setVolMenu--;
      }
    }
  }else{
    UpStat = false;
  }

  if(digitalRead(InDown) == HIGH && (a == 1 || MenuPost > 100)){
    if(DownStat == false){
      change = true;
      DownStat = true;
      timerMenu = 250;
      //a=1;
      if(MenuPost <5){
        MenuPost--;
        if(MenuPost < 0) MenuPost = 2;   
      }else if(MenuPost == 200 && setVolMenu < 25){
        setVolMenu++;
      }//else if(MenuPost == 30){
        
      //}
    }
  }else{
    DownStat = false;
  }
  if(digitalRead(InBack) == HIGH){
    if(!BackStat){
      change = true;
      BackStat = true;
      timerMenu = 250;
      if(a==1){
        a=0;
        lcd.clear();
      }
      else if(a==2){
        lcd.clear();
        switch(MenuPost){
          case 10:
            MenuPost = 0;
            holdMenu = false;
            SerialTest = false;
          break;
          case 20:
            MenuPost = 1;
            holdMenu = false;
            mp3_stop();
          break;
          case 30:
            MenuPost = 2;
            //holdMenu = false;
            dotTest = false;
          break;
        }
        a = 1;
      }
      else if(a == 3){
        lcd.clear();
        switch(MenuPost){
          case 200:
            MenuPost = 20;
            holdMenu = true;
            //mp3.stop();
          break;
          case 300:
            MenuPost = 30;
            //holdMenu = true;
          break;
        }
        a = 2;
      }
    }
  }else{
    BackStat = false;
  }
  if(timerMenu >= 0 && !holdMenu){
    timerMenu--;
    change = true;
  }
}

void menu(){
  switch (MenuPost){
    case 0:
      lcd.setCursor(0,0);
      lcd.print("Menu");
      lcd.setCursor(0,1);
      lcd.print("=> Test Server");
      lcd.setCursor(0,2);
      lcd.print("   Test Sirene");
      lcd.setCursor(0,3);
      lcd.print("   Test Dot-Matriks");
    break;
    case 1:
      lcd.setCursor(0,0);
      lcd.print("Menu");
      lcd.setCursor(0,1);
      lcd.print("   Test Server");
      lcd.setCursor(0,2);
      lcd.print("=> Test Sirene");
      lcd.setCursor(0,3);
      lcd.print("   Test Dot-Matriks");
    break;
    case 2:
      lcd.setCursor(0,0);
      lcd.print("Menu");
      lcd.setCursor(0,1);
      lcd.print("   Test Server");
      lcd.setCursor(0,2);
      lcd.print("   Test Sirene");
      lcd.setCursor(0,3);
      lcd.print("=> Test Dot-Matriks");
    break;
    case 10:
      SerialTest = true;
    break;
    case 20:
      testSirene();
    break;
    case 30:
      //dotTest = true;
      lcd.setCursor(0,0);
      lcd.print("Test DotMatriks");
      lcd.setCursor(0,1);
      lcd.print("> Mohon Tunggu");
      Serial3.println("A");
      delay(500);
    break;
    case 200:
      volSirene();
    break;
    case 300:
      //dotTest = true;
    break;
      
  }
}
