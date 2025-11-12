 
#include "EasyMFRC522.h"
#include <LiquidCrystal.h>


#define MAX_STRING_SIZE 100  
#define BLOCK 1          

int openTime = 2000;

EasyMFRC522 rfidReader(53, 5); 

int button = 22;

bool waitForMasterCard = false;
bool waitForBlankCard = false;

int LCD_RS = 12;
int LCD_ENABLE = 11;
int LCD_D4 = 10;
int LCD_D5 = 9;
int LCD_D6 = 8;
int LCD_D7 = 7;

LiquidCrystal lcd(LCD_RS, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

String removeQuotes(String input) {
  String result = "";
  for (unsigned int i = 0; i < input.length(); i++) {
    if (input[i] != '"') { 
      result += input[i];
    }
  }
  return result;
}


void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Serial.begin(9600);
  Serial.setTimeout(20000);
  rfidReader.init(); 

  pinMode(3, OUTPUT);
 
  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH);

  pinMode(button, INPUT);
  

  lcd.begin(16,2);
  readYourCard();
}



void loop() {

  

  bool success;
  do {
    success = rfidReader.detectTag();    
    delay(50);
    if(!waitForMasterCard) {
      checkForButton();
    }
  } while (!success);

  
  int result;
  char stringBuffer[MAX_STRING_SIZE];

  result = rfidReader.readFile(BLOCK, "mylabel", (byte*)stringBuffer, MAX_STRING_SIZE);

  stringBuffer[MAX_STRING_SIZE-1] = 0;

  bool skipUnknownMsg = false;

  if (result >= 0) {
    bool skipErrMsg = false;
    bool skipEntrance = false;
    
    String data;
    data = removeQuotes(stringBuffer);

    if(waitForMasterCard){
        skipEntrance = true;
        if(data != "MASTER"){
            waitForMasterCard = false;
            readYourCard();
            skipErrMsg = true;
            skipUnknownMsg = true;
        }
    } else if(waitForBlankCard){
        if(data != "MASTER") {
            skipEntrance = true;
            skipErrMsg = true;
            writeOPEN();
            skipUnknownMsg = true;

        } else {
            cloningDenied();
            waitForMasterCard = false;
            waitForBlankCard = false;
            delay(3000);
            skipEntrance = true;
            readYourCard();
        }
    }



    if(data == "OPEN" || data == "MASTER") {

        if(waitForMasterCard) {
            if(data == "MASTER") {
                waitForMasterCard = false;
                waitForBlankCard = true;
                readBlankCard();
            }
            skipEntrance = true;
        } else {
            if(!skipEntrance && !waitForMasterCard && !waitForBlankCard) {
                OPEN();
            }
        }

        

    } else {
        if(!skipErrMsg) {

            Serial.println("Belepes megtagadva!");
            lcd.setCursor(0,0);
            accessDenied();
            delay(openTime);
            lcd.setCursor(0,0);
            readYourCard();
        }
    }
  } else if(success) {
    if(!skipUnknownMsg) {
        unknownCard();
        delay(openTime);
    }
    readYourCard();
  }


  rfidReader.unselectMifareTag();
  delay(100);
}

void checkForButton(){
  int pressed = digitalRead(button);

  if(pressed) {
    waitForMasterCard = true;
    readMasterCard();
  }
}

void writeOPEN(){
    char writeStringBuffer[100];
    int result;
    strcpy(writeStringBuffer, "OPEN");  // you may try a different string here, with LESS than MAX_STRING_SIZE characters
    int stringSize = strlen(writeStringBuffer);
    
    // starting from tag's block #1, writes a data chunk labeled "mylabel", with its content given by stringBuffer, of stringSize+1 bytes (because of the trailing 0 in strings) 
    result = rfidReader.writeFile(BLOCK, "mylabel", (byte*)writeStringBuffer, stringSize+1);

    cloningSuccess();
    delay(3000);

    waitForMasterCard = false;
    waitForBlankCard = false;
    readYourCard();
}

void OPEN(){
  Serial.println("Belepes engedelyezve!");
  lcd.setCursor(0,0);
  accessGranted();

  digitalWrite(3, HIGH);
  delay(openTime);
  digitalWrite(3, LOW);
  lcd.setCursor(0,0);
  readYourCard();
}

void readMasterCard() {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("OLVASD LE A");
    lcd.setCursor(0, 1);
    lcd.print("MASTERT!");
}

void readYourCard(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("OLVASD LE A");
  lcd.setCursor(0, 1);
  lcd.print("KARTYAD!");
}

void accessGranted(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("BELEPES");
  lcd.setCursor(0,1);
  lcd.print("ENGEDELYEZVE!");
}

void accessDenied(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("BELEPES");
  lcd.setCursor(0,1);
  lcd.print("MEGTAGADVA!");
}

void unknownCard(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ISMERETLEN");
  lcd.setCursor(0,1);
  lcd.print("KARTYA!");
}

void readBlankCard() {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MASOLAS...");
    lcd.setCursor(0, 1);
    lcd.print("UJ KARTYA:");
}

void cloningDenied(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("KLONOZAS");
    lcd.setCursor(0, 1);
    lcd.print("SIKERTELEN!");
}

void cloningSuccess(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("KLONOZAS");
    lcd.setCursor(0, 1);
    lcd.print("SIKERES!");
}
