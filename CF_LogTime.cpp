#include <assert.h>
#include <CF_LogTime.h>

// #define DEBUG_LOG_TIME

#define ASCII_0 48  // char(48)


RawLogTime LogTime::raw() {
  uint32_t ms = millis();
  RawLogTime t = {timeBase_sec + (ms / 1000L), (uint16_t) (ms % 1000L)};
  return t;
}

Timestamp LogTime::timestamp() {
  RawLogTime t = raw();
  
  if (t.sec > last_sec) {
    timestampCount = 0;
    last_sec = t.sec;
  } else {
    timestampCount++;
    if (timestampCount == 16) {
      // wait for the next full second to start (with an added safety margin of 1):
      delay(1000 - t.ms + 1);
      t = raw();
      assert(t.sec > last_sec);
      timestampCount = 0;
      last_sec = t.sec;
    }
  }
  Timestamp ts = t.sec << TIMESTAMP_ID_BITS | timestampCount;
  #ifdef DEBUG_LOG_TIME
    Serial.print(F("DEBUG_LOG_TIME: Timestamp "));
    char buf[MAX_TIMESTAMP_STR_LEN];
    Serial.println(formatTimestamp(ts, buf));
  #endif
  return ts;
}


void LogTime::adjust(Timestamp mostRecent) {
  uint32_t mostRecent_sec = mostRecent >> TIMESTAMP_ID_BITS;
  uint32_t current_sec = millis() / 1000L;
  if (mostRecent_sec >= current_sec) {
    timeBase_sec = mostRecent_sec + 1; // continue at the "next" second
  } else {
    reset();
  }
}


void LogTime::reset() {
  timeBase_sec = 0L;
  last_sec = 0L;
  timestampCount = 0;
}


char *formatTimestamp(Timestamp t, char s[MAX_TIMESTAMP_STR_LEN]) {
  uint32_t sec = t >> TIMESTAMP_ID_BITS;
  uint8_t timestampCount = t % 16;
  s[12] = '\0';
  s[11] = ASCII_0 + timestampCount % 10;
  s[10] = ASCII_0 + timestampCount / 10;
  s[9] = '.';
  for(int16_t i = 8; i >= 0; i--) {
    s[i] = ASCII_0 + sec % 10;
    sec /= 10;
  }
  return s;
}

