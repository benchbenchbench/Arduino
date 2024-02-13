

#include <time.h>
//SD card library
#include <SD.h>
//low power library
#include "LowPower.h"
//LCD panel library
#include <LedControl.h>
//#include <SPI.h>
//RTC clock library
#include <DS3231M.h>

//watchdog timer libraries
#include <avr/wdt.h>
#include <avr/interrupt.h>

//#define DEBUGTEXT


DS3231M_Class DS3231M;
File sdFile;
LedControl lc = LedControl(7,6,5,1);
const uint8_t  SPRINTF_BUFFER_SIZE{32};
int chip_select_pin = 10;  

const byte ledPin = 13;
const byte buttonPin = 2;
const byte LCDPin = 8;
const byte SDPin = 4;
const byte clockPin = 9;
const byte tempPin = 3;
const byte A0_pin = A0;
const byte A1_pin = A1;
const byte A2_pin = A2;
volatile byte state = HIGH;

volatile int sleep_count = 0;
const int interval = 15;
const int sleep_total = interval;
bool clockToggle = false;
bool LCDToggle = false;
int sec = 5;
float temp = 0.0;
bool canReadTemp = true;
char finalFileName[] = "00000000.TXT";
int batteryVoltage[5];
byte batteryCheckCounter = 0;


ISR(WDT_vect)
{
  sleep_count ++;
}


void setup() {
  Serial.begin(500000);
  Serial.println("Starting Setup");
  // for whatever reason, this function enables watch dog timer reset instead of interrupt. 
  //wdt_enable(WDTO_8S); // enable 8 second watchdog reset timer

  // this code enables the watchdog timer and only does interrupt
  MCUSR &= ~_BV(WDRF);                            //clear WDRF
  WDTCSR |= _BV(WDCE) | _BV(WDE);                 //enable WDTCSR change
  WDTCSR =  _BV(WDIE) | _BV(WDP3) | _BV(WDP0);    //~8 sec

  pinMode(clockPin, OUTPUT);
  pinMode(LCDPin, OUTPUT);
  pinMode(SDPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(A0_pin, INPUT);
  pinMode(tempPin, OUTPUT);
  pinMode(A2_pin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  // fill battery voltage array for use in the loop
  batteryVoltage[0] = analogRead(A2_pin);
  batteryVoltage[1] = analogRead(A2_pin);
  batteryVoltage[2] = analogRead(A2_pin);
  batteryVoltage[3] = analogRead(A2_pin);
  batteryVoltage[4] = analogRead(A2_pin);

  Serial.print("Initialize voltage:"); Serial.println(batteryVoltage[0]); Serial.println(batteryVoltage[1]); Serial.println(batteryVoltage[2]); Serial.println(batteryVoltage[3]); Serial.println(batteryVoltage[4]); Serial.println(batteryVoltage[5]);
  int batV = GetBatteryVoltage();
  // if batV is greater than 300, we have a battery connected. If batV is less than something small, like 10, then we have USB power because otherwise, this would not be running!
  // there is an edge case where the USB is connected and the battery is also running. Since the UNO doesn't have an easy way to tell which power input is active, we just run with both.
  // However, if the batteries are drained below 300, then we have this odd case where we won't write to the SD card even though there is plenty of power to do so
  
  // put setup into a loop so it can't finish unless there is enough power to do so
  lc.setChar(0,7,char(7),false);
  while(batV < 300 && batV > 10)
  {
    Serial.println(batV);
    batV = GetBatteryVoltage();
    ToggleExternalChipsState(false, false, false, true, false);
    delay(500);
    ToggleExternalChipsState(false, false, false, false, false);
    delay(500);
  }

  // turn on the external chips
  ToggleExternalChipsState(true, true, true, true, true);
   
  delay(250);

  //DisableTime();
  //DisableLCD();

  // turn off external chips
  ToggleExternalChipsState(false, false, false, false, false);
  Serial.println("Ending Setup");
}



// in the loop we want to listen for two events:
// the watch dog timer waking up the sleep funciton on the eighth sleep cycle
// // we want to record to the SD card when this happens
// if the button is pressed
// // we don't want to record to the SD card when this happens
void loop()
{
  #ifdef DEBUGTEXT
  Serial.println("S"); delay(10);
  Serial.println(sleep_count);
  #endif
  int batV = GetBatteryVoltage();
  if((batV < 300 && batV > 10))
  {
    #ifdef DEBUGTEXT
    Serial.println("Voltage too low to continue."); delay(10);
    #endif
    ToggleExternalChipsState(false, false, false, false, false);
    return;
  }

  // if the button is not pressed
  if(state == HIGH)
  {
    canReadTemp = true;
    #ifdef DEBUGTEXT
    Serial.println("A"); delay(10);
    #endif
    //DisableLCD();
    //DisableTime();
    // check the battery voltage often so we can detect low voltage and power down if needed

    ToggleExternalChipsState(false, false, false, false, false);

    //Serial.flush();
    //Serial.end();
    
    attachInterrupt(digitalPinToInterrupt(buttonPin), blink, CHANGE);
    // this uses the least power. About 12mA on Uno R3
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    // this uses a bit more power. About 17mA on Uno R3
    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
    detachInterrupt(digitalPinToInterrupt(buttonPin));

    state = digitalRead(buttonPin);
    // record the time and temp and write to the SD card if we have slept long enough
    if(sleep_count > sleep_total && state == HIGH)
    {
      ToggleExternalChipsState(false, false, true, false, false);
      delay(100); // small delay to ensure the temp sensor stabilizes before being read
      temp = GetTemperature();

      //ToggleExternalChipsState(false, false, true, false, false);
      
      DateTime now = GetTime();
      int RTCtemp = DS3231M.temperature();
      // create the name of the file. Note, the name cannot be any longer than 8 characters for the name and three characters for the extension!!!
      sprintf(finalFileName, "%02d%02d%04d.TXT", now.month(), now.day(), now.year());
      char time[SPRINTF_BUFFER_SIZE];
      FormatTime( time, now);

      ToggleExternalChipsState(false, false, false, false, true);

      
      wdt_reset();
      batV = GetBatteryVoltage();
      if(batV > 250 || batV < 10)
      {
        EnableSDCard();
        
        #ifdef DEBUGTEXT  
        Serial.println(temp);
        Serial.println(RTCtemp);
        #endif

        // battery voltage is returned in the range of 0-1023 by the input pin
        // the voltage divider between the pin and the battery cuts the voltage in half because the max voltage the input pin can read is 5 volts
        WriteTimeTempVoltageToSDCard(finalFileName, time, temp, (5.27 * float(batV))/1023.0 * 2.0);
      }
      

      wdt_reset();
      sleep_count = 0;
    }
    
  }
  else // the button is pressed
  {
    #ifdef DEBUGTEXT
    Serial.println("B"); delay(10);
    #endif
    wdt_reset();
    float tempCheck = temp;
    if((sec%5 == 0) && canReadTemp)
    {
      ToggleExternalChipsState(false, false, true, false, false);
      delay(100); // small delay to ensure the temp sensor stabilizes before being read
      temp = GetTemperature();
      tempCheck = temp;
      #ifdef DEBUGTEXT
      Serial.print("temp:"); Serial.println(temp);
      #endif
      canReadTemp = false;
    }
    // output the battery voltage to the LCD for one second every 5 seconds as the button is held down
    if(sec%5 == 3)
    {
      float volts = 5.27 * float(GetBatteryVoltage())/1023.0 * 2.0;
      tempCheck = volts;
    }
    ToggleExternalChipsState(false, true, true, true, false);
    
    //EnableLCD();
    //EnableTime();
    DateTime now = GetTime();
    sec = now.second();
    
    
    int RTCtemp = DS3231M.temperature();
    #ifdef DEBUGTEXT
    Serial.println(temp);
    Serial.println(RTCtemp);
    #endif
    

    
    //temp = float(batteryVoltage);
    SetLCD(tempCheck, now);
    
    state = digitalRead(buttonPin);
  }
  #ifdef DEBUGTEXT
  Serial.println("E"); delay(10);
  #endif
}

void ToggleExternalChipsState(bool tempState, bool ledState, bool clockState, bool LCDState, bool SDState)
{
  if (tempState) digitalWrite(tempPin, HIGH);
  if (!tempState) digitalWrite(tempPin, LOW);
    
  if (ledState) digitalWrite(ledPin, HIGH);
  if (!ledState) digitalWrite(ledPin, LOW);

  if (clockState && !clockToggle) {digitalWrite(clockPin, HIGH); EnableTime(); clockToggle = true;}
  if (!clockState && clockToggle) {DisableTime(); digitalWrite(clockPin, LOW); clockToggle = false;}

  if (LCDState && !LCDToggle)
  {
    digitalWrite(LCDPin, HIGH);
    EnableLCD();
    LCDToggle = true;
  }
  if(!LCDState && LCDToggle)
  {
    DisableLCD();
    digitalWrite(LCDPin, LOW);
    digitalWrite(5, LOW); // used by the LCD and needs to be disabled in addition to the VCC pin
    LCDToggle = false;
  }

  if (SDState) digitalWrite(SDPin, HIGH);
  if (!SDState) digitalWrite(SDPin, LOW);

}

int GetBatteryVoltage()
{
  int batV = analogRead(A2_pin);
  // if voltage is essentially 0, but the program is still running, then we assume the UNO is powered by the USB plug
  // so don't add average the voltage
  if(batV > 10)
  {
    int index = batteryCheckCounter % 5;
    batteryVoltage[index] = batV;
    int total = 0;
    for (int i = 0; i < 5; i++ )
    {
      total += batteryVoltage[i];
    }
    batteryCheckCounter ++;
    int avgVoltage = total/5;
    #ifdef DEBUGTEXT
    Serial.print("Current Volts:"); Serial.println(batV);
    Serial.print("Index:"); Serial.println(index);
    Serial.print("Total:"); Serial.println(total);
    Serial.print("Avg Volts:"); Serial.println(5.27 * float(avgVoltage)/1023.0 * 2.0);
    #endif
    return avgVoltage;
  }

  return batV;
  
}

void EnableLCD()
{
  lc = LedControl(7,6,5,1);
  lc.shutdown(0,false);
  lc.setIntensity(0,0);
}

void DisableLCD()
{
  lc.shutdown(0,true);
}

void SetLCD(float temperature, DateTime now)
{
  int temp = temperature;
  int fraction = int((temperature - float(temp)) * 10);
  int ones = temp%10;
  temp = temp/10;
  int tens = temp%10;
  temp = temp/10;
  int hundreds = temp;

  int hour = now.hour();
  int hourOnes = hour%10;
  hour = hour/10;
  int hourTens = hour;

  int minutes = now.minute();
  int minutesOnes = minutes%10;
  minutes = minutes/10;
  int minutesTens = minutes;

  lc.setChar(0,7,char(hourTens),false);
  lc.setChar(0,6,char(hourOnes),true);
  lc.setChar(0,5,char(minutesTens),false);
  lc.setChar(0,4,char(minutesOnes),true);
  lc.setChar(0,3,char(hundreds),false);
  lc.setChar(0,2,char(tens),false);
  lc.setChar(0,1,char(ones),true);
  lc.setChar(0,0,char(fraction),false);

}

float GetTemperature()
{
      
      float temperature = analogRead(A0_pin);
      // the LM34DZ will return the voltage as 10mV per degree F. So 65F will be returned from the sensor as 650mV
      // the arduino input pin takes the input voltage and divides it by the max input voltage. So 650mV/5000mV = .13
      // then the arduino takes the final voltage amount and returns it in the 0-1023 int range. So .13 of 1024 is 133
      // my sensor appears to be off by about 3%. This might be related to the damage it took when I repaired the pins which snapped off
      temperature = ((temperature * 4.9)/10.0) * 1.031;
      temperature = float(DS3231M.temperature());
      temperature = (temperature/100.0) * 9.0/5.0 + 32.0;
      return temperature;
}

DateTime GetTime()
{
  return DS3231M.now();
}


void EnableTime()
{
    #ifdef DEBUGTEXT
    Serial.println("Enabling clock"); delay(10);
    #endif
    while (!DS3231M.begin(I2C_FAST_MODE))  // Initialize RTC communications
  {
    Serial.println("Unable to find DS3231M. Checking again in 3s.");
    delay(500);
  }
  #ifdef DEBUGTEXT
  Serial.println("Clock enabled"); delay(10);
  #endif

}

void DisableTime()
{
  Wire.end();
}

void FormatTime(char * formattedTime, DateTime now)
{
    // Use sprintf() to pretty print the date/time with leading zeros
    //char output_buffer[SPRINTF_BUFFER_SIZE];  ///< Temporary buffer for sprintf()
    sprintf(formattedTime, "%04d-%02d-%02d,%02d:%02d:%02d", now.year(), now.month(), now.day(),
    now.hour(), now.minute(), now.second());
    //return output_buffer;
}

void EnableSDCard()
{
    if (SD.begin())
  {
    Serial.println("Initialization Successful. Ready to use");
  } else
  {
    Serial.println("Initialization failed. Please insert an SD card.");
  }
}

void WriteTimeTempVoltageToSDCard(char *name, char time[], float temp, float voltage)
{
  File file = SD.open(name, FILE_WRITE);

  if (file) {    

    file.print(time);
    file.print(",");    
    file.print(temp);
    file.print(",");
    file.print(voltage);
    file.print("\n"); 
    file.close();
    Serial.println("wrote the temp to the SD card");

  }
  else {

    Serial.println("error opening your SD card file. Try again");
    Serial.println(name);

  }
}

void blink()
{
  int currentState = digitalRead(buttonPin);
  if(currentState == LOW)
  {
    state = HIGH;
    //Serial.println("Button Pressed!");
  }
  else
  {
    state = LOW;
    }
  

}


