#include <LiquidCrystal.h>
#include <avr/eeprom.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//#define DEBUG

const int romTotal = 0;

// States
int state = 0;
const int stateFilling = 0;
const int stateEmptying = 1;

// Tank Capacity; 19.5 Imperial Gallons
const float tankCapacity = 19.5;

// Relay sensor pin
int relayPin = A0;
long relayValue;

int counter = 0;

long lastTimes[5] = {-1L, -1L, -1L, -1L, -1L};
long startTime;
long currentTime;

void setup() {
#ifdef ERASEROM
  eeprom_write_block((const void*)0, (void*)romTotal, sizeof(long));
#endif  
  startTime = millis();
  lcd.clear();
  
  // Setup LCD 16rows x 2cols
  lcd.begin(16, 2);
  
  lcd.setCursor(0, 0);
  lcd.print("Bonjour Lucien !");
  lcd.setCursor(0, 1);
  lcd.print("19.5 Imp Gallon");
  
  // Write the cumulative total
  lcd.setCursor(0, 1);
  long totalCap;

  // Delay 10sec to display infos
  delay(10000);
  
  lcd.clear();
}

void loop() {
  // Read relay value
  relayValue = analogRead(relayPin);
  
#ifdef DEBUG
    lcd.setCursor(10, 0);
    static char str[5];
    sprintf(str, "%04d", relayValue);
    lcd.print(str);
#endif
  
  // Current is off; tank is filling
  if (relayValue < 1000) {
    if (state == stateEmptying) {
      startTime = millis();
      state = stateFilling;               
    } else {      
      // Print time
      lcd.setCursor(0, 0);
      currentTime = millis() - startTime;
      lcd.print(timeToString(currentTime/1000));
      
      //Print current G/H
      lcd.setCursor(6, 0);
      unsigned long currentRate = (3600*tankCapacity)/(currentTime/1000);            
      static char rateStr[10];
      sprintf(rateStr, "%6d G/H", currentRate);
      lcd.print(rateStr);

    }
  } else { // Currrent on; tank is emptying
    if (state == stateFilling) {
      if (counter != 0) {
        lastTimes[counter % 5] = currentTime;
      }
      counter++;
      state = stateEmptying;
    } else {  
      long totalTime = 0;
      int totalCount = 0; 
      for (int i=0; i<5; i++) {
        if (lastTimes[i] > 0) {
          totalTime += lastTimes[i];
          totalCount++;
        }
      }
      
      // Calculate & Print the mean
      long meanTime = totalTime/totalCount;                 
      lcd.setCursor(0, 1);
      lcd.print("M=");
      lcd.setCursor(2, 1);
      lcd.print(timeToString(meanTime/1000));      
      
      // Calculate & Print the mean rate
      lcd.setCursor(8, 1);

      long meanRate = (3600*tankCapacity)/(meanTime/1000);            
      static char rateStr[8];
      sprintf(rateStr, "%4.0d G/H", meanRate);
      lcd.print(rateStr);
    }    
  }
}

char * timeToString(unsigned long t) {
  static char str[6];
  long h = t / 3600;
  t = t % 3600;
  int m = t / 60;
  int s = t % 60;
  sprintf(str, "%02d:%02d", m, s);
  return str;
}

