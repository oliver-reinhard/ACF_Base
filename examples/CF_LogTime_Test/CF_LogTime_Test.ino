#include <ArduinoUnitX.h>

#define UNIT_TEST
#include <CF_LogTime.h>

//#define DEBUG_UT_LOGTIME

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  //Test::min_verbosity = TEST_VERBOSITY_ALL;
}

void loop() {
  Test::run();
}

// ------ Unit Tests --------

test(log_timestamp) {
  // ensure this test is not run within the first second of Arduino board time:
  delay(1000);
  uint32_t ms = millis();
  if (ms % 1000 > 500) {
    // sleep until the next full second has started
    delay(1000 - (ms % 1000) + 1);
    ms = millis();
    assertLessOrEqual(ms % 1000, 3);
  }
  uint32_t sec = ms / 1000;
  // 0
  LogTime lt = LogTime();
  Timestamp t1 = lt.timestamp();
  assertEqual(t1>>TIMESTAMP_ID_BITS, sec); // check same second
  assertEqual(t1 & 0xF, 0L); // check identity counter

  // 1 .. 15
  Timestamp t2;
  for(uint32_t i=1; i<=15; i++) {
    t2 = lt.timestamp();
    assertMore(t2, t1);
    assertEqual(t2>>TIMESTAMP_ID_BITS, sec);
    assertEqual(t2 & 0xF, i);
    t1 = t2;
  }
  // 16
  t1 = lt.timestamp();
  // ensure we are in the next full second now
  assertEqual(t1>>TIMESTAMP_ID_BITS, sec+1); 
  assertEqual(t1 & 0xF, 0L);
  
  // 17
  t1 = lt.timestamp();
  assertEqual(t1>>TIMESTAMP_ID_BITS, sec+1);
  assertEqual(t1 & 0xF, 1L);
}

