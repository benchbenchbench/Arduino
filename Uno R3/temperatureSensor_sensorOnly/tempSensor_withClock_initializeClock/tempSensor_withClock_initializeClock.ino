#include <DS3231.h>

DS3231  rtc(SDA, SCL);




void setup()

{  

  rtc.begin();

   rtc.setDOW(FRIDAY);     

  rtc.setTime(22, 52, 15);     

  rtc.setDate(18, 8, 2023);   

}




void loop()

{

}