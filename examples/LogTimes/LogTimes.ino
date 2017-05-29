#include <ACF_LogTime.h>

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect.
  }

  // ensure this test is not run within the first second of Arduino board time:
  delay(1000);
  uint32_t ms = millis();
  if (ms % 1000 > 500) {
    // sleep until the next full second has started
    delay(1000 - (ms % 1000) + 1);
    ms = millis();
  }
  
  uint32_t sec = ms / 1000;
  // 0
  LogTime lt = LogTime();
  Timestamp ts[18];
  uint8_t i = 0;

  // 0 .. 15: timestamps will be within the same second:
  for(i=0; i<=15; i++) {
    ts[i] = lt.timestamp();
  }
  // 16: will be within the next second
  ts[16] = lt.timestamp();
  
  // 17: will be within the same second as 16
  ts[17] = lt.timestamp();

  char str[30];
  for(i=0; i<=17; i++) {
    Serial.print("Timestamp [");
    if (i<10) Serial.print(0);
    Serial.print(i);
    Serial.print("] : ");
    Serial.println(formatTimestamp(ts[i], str));
  }
}

void loop() {
  // do nothing
}
