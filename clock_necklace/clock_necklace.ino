#include <Wire.h>
#include "Adafruit_MCP23017.h"
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"

// Basic pin reading and pullup test for the MCP23017 I/O expander
// public domain!

// Connect pin #12 of the expander to Analog 5 (i2c clock)
// Connect pin #13 of the expander to Analog 4 (i2c data)
// Connect pins #15, 16 and 17 of the expander to ground (address selection)
// Connect pin #9 of the expander to 5V (power)
// Connect pin #10 of the expander to ground (common ground)
// Connect pin #18 through a ~10kohm resistor to 5V (reset pin, active low)

// Output #0 is on pin 21 so connect an LED or whatever from that to ground

Adafruit_MCP23017 mcp;

RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

int lastHour;
int lastMinute;

void setup () {
  while (!Serial); // for Leonardo/Micro/Zero

  Serial.begin(57600);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  Serial.println(rtc.isrunning());

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  DateTime now = rtc.now();

  lastHour = now.hour();
  lastMinute = now.minute();

  mcp.begin();      // use default address 0

  for (int i=0; i < 12; i++){
      mcp.pinMode(i, OUTPUT);
      mcp.digitalWrite(i, LOW);
  }
  
 // mcp.digitalWrite(11, HIGH);
  
  
  printTime();
}

int convertHourToPin(int currentHour) {
  int pin;
  if (currentHour == 24) {
    pin = 0;
  } 
  else if (currentHour < 12) {
    pin = currentHour - 1;
  } 
  else {
    pin = currentHour - 12;
  }
  return pin;
}

int convertMinuteOrSecondToPin(int currentMinute) {
  int pin = (currentMinute / 5);
  if (pin == 12) {  // make sure it won't try to vibrate motor number -1
    pin = 0;
    
  } 
  else if (pin > 12) {
    pin -=  11;
  }
  return pin;
}

int convertCurrentPinToPrevious(int pin) {
  if (pin == 0) {
    pin = 11;
  } else {
    pin -= 1;
  }
  return pin;
}

void startMotorVibration(int pin) {
  mcp.digitalWrite(pin, HIGH);  // maybe more actions will be performed later
}

void stopMotorVibration(int pin) {
  mcp.digitalWrite(pin, LOW);  // maybe more actions will be performed later
}

void cycleMotorVibration(int pin, int duration) {  // duration in seconds
  startMotorVibration(pin);
  delay(duration * 1000);
  stopMotorVibration(pin);
}

void printTime() {
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}

void cycleSeconds(int pin) {
  Serial.println(pin);
  startMotorVibration(pin);  // continuously vibrate seconds motors for testing purposes
  int previousPin = convertCurrentPinToPrevious(pin);
  Serial.println(previousPin);
  stopMotorVibration(previousPin);
}

void checkTime () {
  int pin;
  DateTime now = rtc.now();
  if (lastHour != now.hour()) {  // if the hour has changed, vibrate corresponding motor for 3 seconds
    int currentHour = now.hour();
    lastHour = currentHour; // overwrite lastHour so this won't trigger on next loop
    pin = convertHourToPin(currentHour);
    //cycleMotorVibration(pin, 3);
  }
  if (lastMinute != now.minute()) {  // if the minute has changed, vibrate corresponding motor for 2 seconds
    int currentMinute = now.minute();
    lastMinute = currentMinute;  // overwrite lastMinute so this won't trigger on next loop
    pin = convertMinuteOrSecondToPin(currentMinute);
   // cycleMotorVibration(pin, 2);
  }
  pin = convertMinuteOrSecondToPin(now.second());
  cycleSeconds(pin);
}

void loop () {
  printTime();
  checkTime();  // check time once per second
  delay(1000);
}

