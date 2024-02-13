#include <DS3231M.h>





#include <SD.h>
#include <Wire.h>
#include <SPI.h>

//File data_file;


const int lm35_pin = A0; 

long temperature;  

int chip_select_pin = 10;     //pin 53 for arduino mega 2560

DS3231M_Class DS3231M;
unsigned int year = 2023;
unsigned int month = 03;
unsigned int day = 16;
unsigned int hour = 11;
unsigned int minute = 12;
unsigned int second = 13;



void setup() {
  Serial.println("Starting setup");
  Serial.begin(115200);
  Wire.begin();                                // Start I2C as master device
  //Wire.setClock(I2C_STANDARD_MODE);                     // Set I2C clock speed
  //Wire.beginTransmission(DS3231M_ADDRESS);     // Address the DS3231M
  //uint8_t errorCode = Wire.endTransmission(); 
  //DS3231M.pinSquareWave();
  //DS3231M.adjust(DateTime(year, month, day, hour, minute, second));
  //Serial.print(F("DS3231M chip temperature is "));
  //Serial.println(DS3231M.temperature() / 100.0, 1);  // Value is in 100ths of a degree Celsius
  Serial.println(DS3231M_ADDRESS);


  
  
  
  pinMode(LED_BUILTIN, OUTPUT);




  pinMode(lm35_pin, INPUT);

  Serial.println("end of setup");
  //pinMode(chip_select_pin, OUTPUT);
/*
  if (SD.begin())

  {

    Serial.println("Initialization Successful. Ready to use");

  } else

  {

    Serial.println("Initialization failed. Check your pin connections or change your SD card");

    return;

  }
*/
    

}




void loop() {

  Wire.beginTransmission(DS3231M_ADDRESS); //START, deviceAdr, data direction (write) are queued
  Wire.write(0x0E);                     //Control Register Address is queued. Control flags to various controls on the chip.
  Wire.endTransmission();
  Wire.requestFrom(DS3231M_ADDRESS, (uint8_t)1);
  byte y = Wire.read();
  
  delay(2000);
  Wire.beginTransmission(DS3231M_ADDRESS); //START, deviceAdr, data direction (write) are queued
  Wire.write(0x0E);                     //Control Register Address is queued. Control flags to various controls on the chip.
  Wire.write(0x20);                     //Start Temperature Conversion command byte is queued. This is sets the bit flag to convert the temperature on the chip. Runs once per second.
  Wire.endTransmission();               //queued information are transferred on ACK
  
  delay(2000);
  Wire.beginTransmission(DS3231M_ADDRESS); //START, deviceAdr, data direction (write) are queued
  Wire.write(0x0E);                     //Control Register Address is queued. Control flags to various controls on the chip.
  Wire.endTransmission();
  Wire.requestFrom(DS3231M_ADDRESS, (uint8_t)1);
  byte z = Wire.read();

  Serial.print(y);
  Serial.print(", ");
  Serial.println(z);

  /*
  byte x;
  // we need to await the temperature conversion before proceeding. The bit will stay a value of 1 until it completes, which returns it back to 0.
  do
  {
    Wire.beginTransmission(DS3231M_ADDRESS); //START, deviceAdr, data direction (write) are queued
    Wire.write(0x0E);                     //Control Register Address is queued. Control flags to various controls on the chip.
    Wire.endTransmission();
    Wire.requestFrom(DS3231M_ADDRESS, (uint8_t)1);   //command to the slave to send 1-byte data
    x = Wire.read();                      //data is read from FIFO buffer
    Serial.println(bitRead(x, 5));
    delay(1000);
  }
  while (bitRead(x, 5) != LOW);           //bit-5 of Control Register is LOW indicates end-of-conversion
  */

  Serial.print("Finished Setting control flags");
  Wire.beginTransmission(DS3231M_ADDRESS);        // Address the I2C device
  Wire.write(0x11);                               // Send the register address to read
  uint8_t e = Wire.endTransmission();   // Close transmission
  Wire.requestFrom(DS3231M_ADDRESS, (uint8_t)2);  // Request 1 byte of data
  float temp_u =  (float)Wire.read();  
  byte temp_l =  Wire.read();  
  Serial.print("Temperature is ");
  Serial.print(temp_u);
  Serial.print(".");
  Serial.println(temp_l);
  
  static uint8_t secs;
  DateTime       now = DS3231M.now();  // get the current time from device
  if (secs != now.second())            // Output if seconds have changed
  {
    /*
    Serial.print(now.year());
    Serial.print(", ");
    Serial.print(now.month());
    Serial.print(", ");
    Serial.print(now.day());
    Serial.print(", ");
    Serial.print(now.hour());
    Serial.print(", ");
    Serial.print(now.minute());
    Serial.print(", ");
    Serial.println(now.second());
    secs = now.second();
    */
  }
  temperature = analogRead(lm35_pin);
  digitalWrite(LED_BUILTIN, LOW); 
  //Serial.print("Raw Temp:");
  //Serial.println(temperature);
  temperature = (temperature * 500) / 1023;
  //Serial.print("Scaled Temp:");
  //Serial.println(temperature);



  //data_file = SD.open("test.txt", FILE_WRITE);

  /*
  if (data_file) {    

    Serial.print(rtc.getTimeStr());

    data_file.print(rtc.getTimeStr());

    Serial.print(",");

    data_file.print(",");    

    Serial.println(temperature);

    data_file.println(temperature);

    data_file.close();
    

  }

  

  else {

    Serial.println("error opening your SD card file. Try again");

  }
  */

  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(1000);

}