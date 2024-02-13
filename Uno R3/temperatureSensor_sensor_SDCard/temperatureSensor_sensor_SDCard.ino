#include <LedControl.h>

#include <DS3231M.h>
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include "lowPower.h"

File data_file;
DS3231M_Class DS3231M;
LedControl lc = LedControl(7,6,5,1);
unsigned long delaytime=500;

const int lm35_pin = A0; 
const int BUTTON_PIN = 9; 
int currentState;
float temperature = 0;
unsigned long myTime;
int chip_select_pin = 10;     //pin 53 for arduino mega 2560
const uint8_t  SPRINTF_BUFFER_SIZE{32};
uint8_t secs;
bool canWrite = true;
char name_buffer[SPRINTF_BUFFER_SIZE];


void setup() {
  Serial.println("Starting setup");
  //pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  while (!DS3231M.begin())  // Initialize RTC communications
  {
    Serial.println(F("Unable to find DS3231M. Checking again in 3s."));
    delay(3000);
  }                         // of loop until device is located

  DS3231M.pinSquareWave();  // Make INT/SQW pin toggle at 1Hz
  //DS3231M.adjust();  // Set to library compile Date/Time
  




  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(lm35_pin, INPUT);

  pinMode(chip_select_pin, OUTPUT);
  //DateTime       now = DS3231M.now();  // get the current time from device
  
  if (SD.begin())

  {

    Serial.println("Initialization Successful. Ready to use");

  } else

  {

    Serial.println("Initialization failed. Please insert an SD card.");

    return;

  }

    ///< Temporary buffer for sprintf()
  //sprintf(name_buffer, "%02d-%02d-%04d.txt", now.month(), now.day(), now.year());
  //Serial.println(name_buffer);
  /* The MAX72XX is in power-saving mode on startup, we have to do a wakeup call */
  lc.shutdown(0,true);

  /* Set the brightness to a medium values */
  lc.setIntensity(0,0);

  /* and clear the display */
  lc.clearDisplay(0);

  Serial.println("end of setup");
}




void loop() {

  
  DateTime       now = DS3231M.now();  // get the current time from device
  currentState = digitalRead(BUTTON_PIN);
  //if (secs == now.second())            // Output if seconds have changed
  if (now.second()%59 == 0)
  {
    
    if(canWrite)
    {
      lc.shutdown(0,true);

      // Use sprintf() to pretty print the date/time with leading zeros
      char output_buffer[SPRINTF_BUFFER_SIZE];  ///< Temporary buffer for sprintf()
      sprintf(output_buffer, "%04d-%02d-%02d,%02d:%02d:%02d", now.year(), now.month(), now.day(),
              now.hour(), now.minute(), now.second());
      //Serial.println(output_buffer);
      temperature = analogRead(lm35_pin);
      temperature = (((temperature) * 500.0) / 1023.0);
      secs = now.second();  // Set the counter variable
      
      //Serial.print("Raw Temp:");
      //Serial.println(temperature);
      

      //Serial.print("Scaled Temp:");
      //Serial.println(temperature);

      //Serial.println(name_buffer);

      data_file = SD.open("TEST.TXT", FILE_WRITE);
      if (data_file) {    
        //myTime = millis()/1000;
        data_file.print(output_buffer);
        data_file.print(",");    
        data_file.print(temperature);
        data_file.print("\n"); 
        data_file.close();
        //Serial.println("wrote the temp to the SD card");

      }
      else {

        Serial.println("error opening your SD card file. Try again");

      }
      canWrite = false;
      

      
    }

      if(currentState == LOW)
      {
        //Serial.println("Button pressed!");
        lc.shutdown(0,false);
      }
      else
      {
        lc.shutdown(0,true);
      }
    
    
    //delay(1000);
    
  }
  else
  {
    //lc.shutdown(0,false);
    if(currentState == LOW)
      {
        //Serial.println("Button pressed!");
        
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
        
        //lc.shutdown(0,false);
        
        lc.setChar(0,7,char(hourTens),false);
        lc.setChar(0,6,char(hourOnes),true);
        lc.setChar(0,5,char(minutesTens),false);
        lc.setChar(0,4,char(minutesOnes),true);
        lc.setChar(0,3,char(hundreds),false);
        lc.setChar(0,2,char(tens),false);
        lc.setChar(0,1,char(ones),true);
        lc.setChar(0,0,char(fraction),false);
        
        lc.shutdown(0,false);
      }
      else
      {
        lc.shutdown(0,true);
      }


    //delay(1000);
    //secs = now.second();  // Set the counter variable
    canWrite = true;
  }


  //delay(1000);
  //digitalWrite(LED_BUILTIN, LOW); 
  //delay(1000);

}