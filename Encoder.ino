#include "Encoder.h"
#include <Wire.h> 
#include "LiquidCrystal_I2C.h"
#include "DS3231.h"
#include "RotaryEncoder.h"
#include "DHT.h"


#define DHT1          2
#define DHTTYPE       DHT22

#define HOME_MENU         4
#define SET_MENU          5 

#define TEMPERATURE_MENU  0
#define HUMIDITY_MENU     1
#define DATE_TIME_MENU    2
#define CALIBER_MENU      3


RotaryEncoder encoder(A2, A3);
LiquidCrystal_I2C lcd(0x27,20,4);
DS3231 clock;
DHT sensor1(DHT1, DHTTYPE);
RTCDateTime dt;

bool  buttonMinus, buttonPlus, EncPushSW;
const byte buttonEnter = 5; // Encoder Press Switch
static int pos = 0;
static int dir = 0;
static int CursorFreez = 0;

float temp, ActTemp;
float humidity = 0, ActHumi = 0;
int menu_idx = HOME_MENU;
int  prev_menu_idx = HOME_MENU;
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
int buttonState, buttonState_1, buttonState_2;
int lastButtonState = LOW, lastButtonState_1 = LOW, lastButtonState_2 = LOW;
unsigned long lastDebounceTime = 0, lastDebounceTime_1 = 0, lastDebounceTime_2 = 0;
unsigned long debounceDelay = 50;
char curPos = 0, PrevcurPos = 0;
char curPos_subMenu = 0, PrevcurPos_subMenu = 0;
char menuLevel[4] = {2, 2, 2, 3};

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
              
          else if (menu_idx == SET_MENU) {
          switch(curPos) {
            case 0:
            menu_idx = TEMPERATURE_MENU; // Temperature
            break;
            case 1:
            menu_idx = HUMIDITY_MENU;    // Humidity Menu
            break;
            case 2:
            menu_idx = DATE_TIME_MENU;  // Date & Time
            break;
            case 3:
            menu_idx = CALIBER_MENU;    // Caliberation
            break;
            case 4:
            menu_idx = HOME_MENU;
            break;
         }
          
        }                                
      }
    }
  }
lastButtonState = reading;

 if( menu_idx == SET_MENU) {
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
   } else if((menu_idx == TEMPERATURE_MENU ||
             menu_idx == HUMIDITY_MENU || 
             menu_idx == DATE_TIME_MENU || 
             menu_idx == CALIBER_MENU) && CursorFreez) {
       if(dir == 1) {
     if (curPos_subMenu > menuLevel[menu_idx])
         curPos_subMenu = menuLevel[menu_idx]; 
     else       
         curPos_subMenu++; 
     } else if (dir == -1) {
        if (curPos_subMenu <= 0)
            curPos_subMenu = 0;
        else        
            curPos_subMenu--; 
      } 
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
  } else if((menu_idx == DATE_TIME_MENU) || 
            (menu_idx == TEMPERATURE_MENU) ||
            (menu_idx == HUMIDITY_MENU) ||
            (menu_idx == CALIBER_MENU) && CursorFreez) {
     if (PrevcurPos_subMenu != curPos_subMenu) {
          lcd.clear();
          PrevcurPos_subMenu = curPos_subMenu;
      }
      if (curPos_subMenu < menuLevel[menu_idx] ) {
          lcd.setCursor(0, curPos_subMenu);
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
  int reading;
  char dir;
  
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
    lcd.print("BACK");
    break;
    case TEMPERATURE_MENU:
    lcd.setCursor(1, 0);
    lcd.print("TEMPERATURE_MENU");
    lcd.setCursor(1, 1);
    lcd.print("BACK");
    break;
    case HUMIDITY_MENU:
    lcd.setCursor(1, 0);
    lcd.print("HUMIDITY_MENU");
    lcd.setCursor(1, 1);
    lcd.print("BACK");
    break;
    case CALIBER_MENU:
    /* Sensor read temperature Value (Measured value) */
    lcd.setCursor(1, 0);
    lcd.write(1);
    AddSpace(1);
    lcd.print(temp, 1);
    lcd.write(3);
    lcd.print("C");
    AddSpace(2);
    lcd.write(1);
    AddSpace(1);
    if(curPos_subMenu == 0) {
    reading = digitalRead(buttonEnter);
    if (reading != lastButtonState_1) {
      lastDebounceTime_1 = millis();
    }
    if ((millis() - lastDebounceTime_1) > debounceDelay) {
    if (reading != buttonState_1) {
      buttonState_1 = reading;
      if (buttonState_1 == HIGH) {
          CursorFreez = !CursorFreez;
          Serial.print("Temp : CursorFreez = ");
          Serial.println(CursorFreez);
        }
      }
     }
        lastButtonState_1 = reading;
        dir = encoder.getDirection();
        if (!CursorFreez) {
        if (dir == 1) {
            ActTemp = ActTemp + 0.5;
        }
        else if (dir == -1) {
            ActTemp = ActTemp - 0.5;;  
        }
       }
    } 
    lcd.setCursor(13, 0);
    lcd.print(ActTemp, 1);
    lcd.write(3);
    lcd.print("C");
    /* Sensor read humidity Value (Measured value) */
    lcd.setCursor(1, 1);
    lcd.write(2);
    AddSpace(1);
    lcd.print(humidity, 1);
    lcd.print("%");
    AddSpace(3);
    lcd.write(2);

    if(curPos_subMenu == 1) {
       reading = digitalRead(buttonEnter);
    if (reading != lastButtonState_2) {
      lastDebounceTime_2 = millis();
    }
    if ((millis() - lastDebounceTime_2) > debounceDelay) {
    if (reading != buttonState_2) {
      buttonState_2 = reading;
      if (buttonState_2 == HIGH) {
          CursorFreez = !CursorFreez;
         Serial.print("Humi : CursorFreez = ");
            Serial.println(CursorFreez);
        }
      }
    }
    lastButtonState_2 = reading;
    dir = encoder.getDirection();
    if (!CursorFreez) {
        if (dir == 1) {
            ActHumi = ActHumi + 0.5;
        }
        else if (dir == -1) {
            ActHumi = ActHumi - 0.5; 
        }
      }        
    }
    lcd.setCursor(13, 1);
    lcd.print(ActHumi, 1);
    lcd.print("%");
    lcd.setCursor(1, 2);
    lcd.print("BACK");
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
  lcd.print("Home");
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
