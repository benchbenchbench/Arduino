const byte ledPin = 13;
const byte mosfet = 9;

void setup() {
  // put your setup code here, to run once:
  
  //digitalWrite(ledPin, LOW);
  Serial.begin(500000);
  Serial.println("Starting Setup");

  pinMode(ledPin, OUTPUT);
  pinMode(mosfet, OUTPUT);

  Serial.println("Ending Setup");


}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(ledPin, HIGH);
  digitalWrite(mosfet, 1);
  delay(1000);
  digitalWrite(ledPin, LOW);
  digitalWrite(mosfet, LOW);
  delay(1000);
}
