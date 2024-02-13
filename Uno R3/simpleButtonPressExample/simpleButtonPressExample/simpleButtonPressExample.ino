const int BUTTON_PIN = 9; 
int lastState = HIGH;
int currentState;


void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
   pinMode(BUTTON_PIN, INPUT_PULLUP);
   Serial.println("Starting button test.");
}

void loop() {
  // put your main code here, to run repeatedly:
  currentState = digitalRead(BUTTON_PIN);
  if(currentState == LOW)
  {
    Serial.println("Button pressed!");
    
  }
}
