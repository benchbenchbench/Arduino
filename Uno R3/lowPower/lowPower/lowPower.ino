#include <time.h>
#include "LowPower.h"

const byte ledPin = 13;
const byte interruptPin = 2;
volatile byte state = LOW;


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);
}

void blink()
{
  state = !state;

}

void loop2() {
  // put your main code here, to run repeatedly:
  
  //delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.println("Power Down");
  //LowPower.idle(SLEEP_4S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
  LowPower.powerStandby(period_t period, ADC_OFF, BOD_OFF)
  
  //LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
  //delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Power On");
  delay(4000);
  //LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
  

}

void loop()
{
  digitalWrite(ledPin, state);

}
