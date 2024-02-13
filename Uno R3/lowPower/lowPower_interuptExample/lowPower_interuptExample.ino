

#include <time.h>
#include "LowPower.h"
#include <LedControl.h>
#include <SPI.h>
#include <DS3231M.h>

DS3231M_Class DS3231M;
LedControl lc = LedControl(7,6,5,1);

const byte ledPin = 13;
const byte interruptPin = 2;
volatile byte state = HIGH;


void setup() {
  Serial.println("Starting Setup");
  
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);
  //digitalWrite(ledPin, LOW);
  Serial.begin(500000);
  pinMode(interruptPin, INPUT_PULLUP);

  lc.shutdown(0,true);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,0);
  lc.setChar(0,7,'7',false);
  
  while (!DS3231M.begin())  // Initialize RTC communications
  {
    Serial.println(F("Unable to find DS3231M. Checking again in 3s."));
    delay(3000);
  }                         // of loop until device is located

  DS3231M.pinSquareWave();  // Make INT/SQW pin toggle at 1Hz
  Serial.println("Ending Setup");
}

void loop()
{
  Serial.println("S"); delay(10);
  // if the button is not pressed
  if(state == HIGH)
  {
    Serial.println("A"); delay(10);
    lc.shutdown(0,true);
    DateTime       now = DS3231M.now();  // get the current time from device
    Serial.println(now.second());
    digitalWrite(ledPin, LOW);
    attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);
    // this uses the least power. About 12mA on Uno R3
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); 
    // this uses a bit more power. About 17mA on Uno R3
    //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
    detachInterrupt(digitalPinToInterrupt(interruptPin));
    state = digitalRead(interruptPin);
  }
  else // the button is pressed
  {
    Serial.println("B"); delay(10);
    lc.shutdown(0,false);
    digitalWrite(ledPin, HIGH);
    state = digitalRead(interruptPin);
  }
  Serial.println("E"); delay(10);
}

void blink()
{
  int currentState = digitalRead(interruptPin);
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

void loop2() {
  // put your main code here, to run repeatedly:
  
  //delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.println("Power Down");
  //LowPower.idle(SLEEP_4S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
  LowPower.powerStandby(SLEEP_4S, ADC_OFF, BOD_OFF);
  
  //LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
  //delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Power On");
  delay(4000);
  //LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
  

}


