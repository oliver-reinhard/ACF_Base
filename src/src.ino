
#define SOS_LED_PIN LED_BUILTIN
    
void setup() {
  pinMode(SOS_LED_PIN, OUTPUT);
}

void loop() {
  // never returns:
  blink_long_S_O_S();
}

/*
 * Infinite loop, never ends.
 */
void blink_S_O_S() {
  int pulse = 300; // [ms]
  while(1) {
    // S.O.S. = . . . – – – . . .
    pulse = (pulse + 200) % 400; // toggles between 100 and 300 ms
    delay(pulse);
    for(byte i=0; i<3; i++) {
      digitalWrite(SOS_LED_PIN, HIGH);
      delay(pulse);
      digitalWrite(SOS_LED_PIN, LOW);
      delay(pulse); 
    }      
  }
}

/*
 * Infinite loop, never ends.
 */
void blink_long_S_O_S() {
  int pulse = 600; // [ms]
  while(1) {
    // S.O.S. = . . . – – – . . .
    pulse = (pulse + 400) % 800; // toggles between 200 and 600 ms
    delay(pulse);
    for(byte i=0; i<3; i++) {
      digitalWrite(SOS_LED_PIN, HIGH);
      delay(pulse);
      digitalWrite(SOS_LED_PIN, LOW);
      delay(pulse); 
    }      
  }
}
