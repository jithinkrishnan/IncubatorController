#include "Encoder.h"
#include <Wire.h> 
#include "LiquidCrystal_I2C.h"
#include "DS3231.h"
#include "RotaryEncoder.h"
#include "DHT.h"


#define DHT1          2
#define DHTTYPE       DHT22

#define HOME_MENU         0  
#define DATE_TIME_MENU    2
#define TEMPERATURE_MENU  3
#define HUMIDITY_MENU     4
#define CALIBER_MENU      5
#define SET_MENU          1

RotaryEncoder encoder(A2, A3);
LiquidCrystal_I2C lcd(0x27,20,4);
DS3231 clock;
DHT sensor1(DHT1, DHTTYPE);
RTCDateTime dt;

bool  buttonMinus, buttonPlus, EncPushSW;
const byte buttonEnter = 5; // Encoder Press Switch
static int pos = 0;
static int dir = 0;

float temp;
float humidity;
int menu_idx = HOME_MENU;
int  prev_menu_idx = HOME_MENU;
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
int buttonState;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
char curPos = 0, PrevcurPos = 0;

uint8_t CharTemp[8] = {B01110, B01010, B01010, B01110, B01110, B11111, B11111, B01110};
uint8_t CharHumi[8] = {B00100, B01110, B11111, B00100, B10001, B00100, B10001, B00100};
uint8_t CharDegre[8]  = {B00110, B01001, B01001, B00110, B00000, B00000, B00000, B00000};
uint8_t CharArrow[8] = {B00000, B00000, B10000, B10000, B10111, B10011, B10101, B01000};

void setup() {
  
 
   // You may have to modify the next 2 lines if using other pins than A2 and A3
  PCICR |= (1 << PCIE1);    // This enables Pin Change Interrupt 1 that covers the Analog input pins or Port C.
  PCMSK1 |= (1 << PCINT10) | (1 << PCINT11);  // This enables the interrupt for pin 2 and 3 of Port C.
  pinMode (buttonEnter, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcd.createChar(1, CharTemp);
  lcd.createChar(2, CharHumi);
  lcd.createChar(3, CharDegre);
  lcd.createChar(4, CharArrow);
  clock.begin();
  Serial.begin(115200);
  //clock.setDateTime(2019, 4, 13, 19, 21, 00);
  lcd.clear();
  sensor1.begin();
}

long oldPosition  = -999;

ISR(PCINT1_vect) {
  encoder.tick(); // just call tick() to check the state.
}
void loop() {
  
   enc_fun();
   menu(menu_idx);
   ShowCursor();
}

void enc_fun () {
   int newPos = encoder.getPosition();
   int reading = digitalRead(buttonEnter);
   dir = encoder.getDirection();
   
 if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
          if (menu_idx == HOME_MENU)
              menu_idx = SET_MENU;
          else if (menu_idx == SET_MENU && (curPos == 4)) // Exit from Settings menu
                   menu_idx = HOME_MENU;
          else if (menu_idx == SET_MENU && (curPos == 0)) // Temperature
                   menu_idx = TEMPERATURE_MENU;
          else if (menu_idx == SET_MENU && (curPos == 1)) // Humidity Menu
                   menu_idx = HUMIDITY_MENU;
          else if (menu_idx == SET_MENU && (curPos == 2)) // Date & Time
                   menu_idx = DATE_TIME_MENU;
          else if (menu_idx == SET_MENU && (curPos == 3)) // Caliberation
                   menu_idx = CALIBER_MENU;                                 
      }
    }
  }
lastButtonState = reading;

   if(dir == 1) {
     if (curPos > 4)
         curPos = 4; 
     else       
         curPos++; 
     } else if (dir == -1) {
        if (curPos <= 0)
            curPos = 0;
        else        
            curPos--; 
    } 
}


void ShowCursor() {
  if (menu_idx == SET_MENU) {
      if (PrevcurPos != curPos) {
          lcd.clear();
          PrevcurPos = curPos;
      }
      if (curPos < 4 ) {
        lcd.setCursor(0, curPos);
        lcd.print(">");
      } else {
        lcd.setCursor(13, 3);
        lcd.print(">");
      }
  } else if( (menu_idx == DATE_TIME_MENU) || (menu_idx == TEMPERATURE_MENU) || (menu_idx == HUMIDITY_MENU ) || (menu_idx == CALIBER_MENU) ) {
     if (PrevcurPos != curPos) {
          lcd.clear();
          PrevcurPos = curPos;
      }
      if (curPos < 2 ) {
        lcd.setCursor(0, 1);
        lcd.print(">");
      } else {
        lcd.setCursor(1, 1);
        lcd.print(">");
      }
  }
      
}
void showclock() {
  dt = clock.getDateTime();
  char buf[20];
  sprintf(buf, "%2d:%2d:%2d %2d/%2d/%d",dt.hour, dt.minute, dt.second, dt.day, dt.month, dt.year);
  lcd.print(buf);
}

void showTempHumidity() {
    currentMillis = millis();
  
  if (currentMillis - previousMillis > 1000) {
      temp = sensor1.readTemperature();
      humidity = sensor1.readHumidity();
      previousMillis = currentMillis;
  }
  
  lcd.write(1);
  AddSpace(1);
  lcd.print(temp, 1);
  lcd.write(3);
  lcd.print("C");
  AddSpace(2);
  lcd.write(2);
  AddSpace(1);
  lcd.print(humidity, 1);
  lcd.print("%");
}

void menu(char menu_idx) {
  if(prev_menu_idx!= menu_idx) {
      lcd.clear();
      prev_menu_idx = menu_idx;
  }
      
  switch(menu_idx) {
    case HOME_MENU:
    homemenu();
    break;
    case SET_MENU:
    Settings();     
    break;
    case DATE_TIME_MENU:
    lcd.setCursor(1, 0);
    lcd.print("DATE_TIME_MENU");
    lcd.setCursor(1, 1);
    lcd.print("EXIT");
    break;
    case TEMPERATURE_MENU:
    lcd.setCursor(1, 0);
    lcd.print("TEMPERATURE_MENU");
    lcd.setCursor(1, 1);
    lcd.print("EXIT");
    break;
    case HUMIDITY_MENU:
    lcd.setCursor(1, 0);
    lcd.print("HUMIDITY_MENU");
    lcd.setCursor(1, 1);
    lcd.print("EXIT");
    break;
    case CALIBER_MENU:
    lcd.setCursor(1, 0);
    lcd.print("CALIBER_MENU");
    lcd.setCursor(1, 1);
    lcd.print("EXIT");
    break;
    
  }
}

void Settings() {
  lcd.setCursor(1, 0);
  lcd.print("Temperature");
  lcd.setCursor(1, 1);
  lcd.print("Humidity");
  lcd.setCursor(1, 2);
  lcd.print("Date & Time");
  lcd.setCursor(1, 3);
  lcd.print("Calibration");
  lcd.setCursor(14, 3);
  lcd.print("Exit");
}

void homemenu(void) {
  lcd.setCursor(0, 0);
  showclock();
  lcd.setCursor(0, 1);
  showTempHumidity();
  
}

void AddSpace(char no) {
  char i;
  for(i = 0; i < no; i++) {
    lcd.print(" ");
  }
}
