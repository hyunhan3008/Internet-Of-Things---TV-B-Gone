// MyThing.ino
// COM3505 lab exercises sketch template

int pushButton = 14, externalLEDR = 32, externalLEDY = 15, externalLEDG = 12;
uint8_t buttonState = 0; 

// initialisation entry point
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(115200);
  pinMode(pushButton, INPUT_PULLUP);
  pinMode(externalLEDR, OUTPUT);
  pinMode(externalLEDY, OUTPUT);
  pinMode(externalLEDG, OUTPUT);
}

// task loop entry point
void loop() {

  buttonState = digitalRead(pushButton);

  if (buttonState == LOW) {
    // turn LED on:
  digitalWrite(externalLEDR, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(externalLEDR, LOW);    // turn the LED off by making the voltage LOW
  delay(500);
  digitalWrite(externalLEDY, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(externalLEDY, LOW);    // turn the LED off by making the voltage LOW
  delay(500);
  digitalWrite(externalLEDG, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(externalLEDG, LOW);    // turn the LED off by making the voltage LOW
  delay(500);
  } else {
    
  digitalWrite(externalLEDR, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(5000);                       // wait for a second
  digitalWrite(externalLEDR, LOW);    // turn the LED off by making the voltage LOW
  delay(5000);
  digitalWrite(externalLEDY, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(5000);                       // wait for a second
  digitalWrite(externalLEDY, LOW);    // turn the LED off by making the voltage LOW
  delay(2000);
  digitalWrite(externalLEDG, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(2000);                       // wait for a second
  digitalWrite(externalLEDG, LOW);    // turn the LED off by making the voltage LOW
  delay(500);
  }
  
 
}
