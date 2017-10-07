//Must be downloaded from here: 
//https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/
#include <LiquidCrystal_I2C.h>  
#include <Wire.h>

//Arduino
//A4 to SDA
//A5 to SCL
//ESP8266
//GPIO 4 to SDA
//GPIO 5 is SCL
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //Set the LCD I2C address



void setup() {
  // put your setup code here, to run once:
  lcd.begin(20,4);
  lcd.setCursor(3,0);
  lcd.print("Test A");
  lcd.setCursor(8,1);
  lcd.print("****");
  lcd.setCursor(0,2);
  lcd.print("This is a demo text");
  lcd.setCursor(8,3);
  lcd.print("****");
}

void loop() {
  // put your main code here, to run repeatedly:
//  lcd.print("Test");
  
}
