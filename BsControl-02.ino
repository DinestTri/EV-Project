/*
   LCD Display 1602A module
*/

#include <Wire.h>
#include <LiquidCrystal.h>
#include <SparkFunBQ27441.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Pushbutton interface--------------
byte lastButtonState = LOW;
byte currentButtonState = LOW;
unsigned long lastButtonDebounceTime = 0;
unsigned long buttonDebounceDelay = 20;
unsigned int PressCount = 0;
#define BUTTON_PIN 14

//Babysitter interface--------------
// Set BATTERY_CAPACITY to the design capacity of your battery.
const unsigned int BATTERY_CAPACITY = 1000; //800; //850; // e.g. 850mAh battery
const int CE_PIN = 8;
int OnCount = 0;
int BattCharging = 0;
int bs_CE_on = 0; //to keep the status of CE


void setupBQ27441(void)
{
  if (!lipo.begin()) // true if  successful
  {
    lcd.setCursor(0, 1);
    lcd.print("Babysitter error  ");
    delay(2000);
    while (1) ;
  }
  lcd.setCursor(16, 2);
  lcd.print("BabysitterConnOK  ");
  delay(2000);

  // Uset lipo.setCapacity(BATTERY_CAPACITY) to set the design capacity of your battery.
  lipo.setCapacity(BATTERY_CAPACITY);
  delay(1000);
}

void setup()
{
  lcd.begin(16, 2);
  //lcd.setCursor(0,0);
  lcd.print("GEEKS EV Chargng");
  lcd.setCursor(0, 1);
  lcd.print("BITS Capstone");
  pinMode(BUTTON_PIN, INPUT);
  pinMode(CE_PIN, OUTPUT);
  digitalWrite(CE_PIN, HIGH);
  delay(2000);
  Serial.begin(9600); //RPi interface

  setupBQ27441(); //babysitter interface

  delay(2000);
}

void printBatteryStats()
{
  // Read battery stats from the BQ27441-G1A
  unsigned int soc = lipo.soc();  // Read state-of-charge (%)
  unsigned int volts = lipo.voltage(); // Read battery voltage (mV)
  int current = lipo.current(AVG); // Read average current (mA)
  unsigned int fullCapacity = lipo.capacity(FULL); // Read full capacity (mAh)
  unsigned int capacity = lipo.capacity(REMAIN); // Read remaining capacity (mAh)
  int power = lipo.power(); // Read average power draw (mW)
  int health = lipo.soh(); // Read state-of-health (%)

  // Now print out those values:
  String toPrint = String(soc) + ",";
  toPrint += String(volts) + ",";
  toPrint += String(current) + ",";
  toPrint += String(capacity) + ",";
  toPrint += String(fullCapacity) + ",";
  toPrint += String(power) + ",";
  toPrint += String(health);

  Serial.println(toPrint);
  
}

void printSoC()
{
  unsigned int soc = lipo.soc();  // Read state-of-charge (%)
    
  // Now print out those values:
  String toPrint = String(soc);

  Serial.write(soc);
}


void loop()
{
  //Output to LCD
  byte readValue = digitalRead(BUTTON_PIN);

  if (readValue != lastButtonState) {
    lastButtonDebounceTime = millis();
  }

  if (millis() - lastButtonDebounceTime > buttonDebounceDelay) {
    if (readValue != currentButtonState) {
      currentButtonState = readValue;
      if (currentButtonState == HIGH) {
        lcd.print("Pressed  ");
        PressCount++;
        if (PressCount > 4)
          PressCount = 0;
      }
    }
  }

  lastButtonState = readValue;
  if (PressCount > 0)
  {
    lcd.print(PressCount);
    switch (PressCount) {
      case 1:
        lcd.begin(16, 2);
        lcd.print("RPi ip: Start  ");
        lcd.setCursor(0, 1);
        lcd.print("Started EV Charg");
        Serial.write(11);
        digitalWrite(CE_PIN, LOW);
        delay(100);
        bs_CE_on = 1;
        break;
      case 2:
        lcd.begin(16, 2);
        lcd.print("RPi ip: Stop  ");
        lcd.setCursor(0, 1);
        lcd.print("Stopped EV Charg ");
        Serial.write(12);
        digitalWrite(CE_PIN, HIGH);
        delay(100);
        bs_CE_on = 0;
        break;
      case 3:
        lcd.begin(16, 2);
        lcd.print("RPi ip: Read SoC  ");
        lcd.setCursor(0, 1);
        lcd.print("Read EV Batt SoC ");
        Serial.write(13);
        bs_CE_on = 1;
        break;
      case 4:
        lcd.begin(16, 2);
        lcd.print("RPi ip: Reset  ");
        lcd.setCursor(0, 1);
        lcd.print("Reset EV Chargng ");
        Serial.write(14);
        bs_CE_on = 0;
        break;
      default:
        // wrong pin number, do nothing
        // all LEDs will be powered off
        break;
    }
  }

  //Interaction with RaspberriPy...
  //BatteryControl.py is interacting with BsControl.ino
  if (Serial.available() > 0) {
    unsigned int soc=0;
    int inputCommand = Serial.read() - '0';
    lcd.print(inputCommand);
    switch (inputCommand) {
      case 1:
        lcd.begin(16, 2);
        lcd.print("User Cmd: Start  ");
        digitalWrite(CE_PIN, LOW);
        delay(100);
        bs_CE_on = 1;
        ////soc = lipo.soc();  // Read state-of-charge (%)        
        //lcd.print(soc);
       //// Serial.write(soc);        
        break;
      case 2:
        lcd.begin(16, 2);
        lcd.print("User Cmd: Stop  ");
        lcd.setCursor(0, 1);
        lcd.print("Stopped EV Charg ");
        digitalWrite(CE_PIN, HIGH);
        delay(100);
        bs_CE_on = 0;
        break;
      case 3:
        lcd.begin(16, 2);
        lcd.print("User Cmd: Read  ");
        soc = lipo.soc();  // Read state-of-charge (%)        
        lcd.setCursor(0, 1);
        lcd.print(soc);
       //// Serial.write(soc);
        bs_CE_on = 1;
        break;
      case 4:
        lcd.begin(16, 2);
        lcd.print("RPi ip: Reset  ");
        lcd.setCursor(0, 1);
        lcd.print("Reset EV Chargng ");
        //Serial.write(14);
        //bs_CE_on = 0;
        break;
      default:
      ////soc = lipo.soc();  // Read state-of-charge (%) 
     //// Serial.write(soc);
        // wrong pin number, do nothing
        // all LEDs will be powered off
        break;
    }

  }

  printBatteryStats();
  ////printSoC();


  //delay(1000);
}
