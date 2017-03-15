#ifndef ACF_LOGTIME_H_INCLUDED
  #define ACF_LOGTIME_H_INCLUDED
  
  #include <Arduino.h>

   
  /**
   * Timestamps are unique identifiers across the full time range of this type.
   * 
   * Timestamp format: 28 bits [seconds since last log reset] + 4 bits [uniquness identifier 0..15].
   *                   - 2^28 seconds is about 8.5 years
   *                   - the log reset may have happened several board resets earlier
   *                   - up to 16 timestamps can be generated within one second
   * The timestamp() function creates ascending values in strong monotony, even across board resets.
   */
  typedef uint32_t Timestamp;
  
  #define TIMESTAMP_ID_BITS   4
  #define UNDEFINED_TIMESTAMP 0L
  
  struct RawLogTime {
    uint32_t sec; // seconds since log was last reset or since last board reset, which ever happened earlier
    uint16_t ms; // milliseconds [0 .. 999]
  };

  
  class LogTime {

    public:
      
      /**
       * Returns a unique timestamp. A maximum of 16 timestamps can be generated within the same second.
       * 
       * Note: this function will delay() until the next second starts (where millis() % 1000 == 0) if the 
       * maximum of 16 timestamps in a given second is exceeded.
       */
      Timestamp timestamp();
      
      /**
       * Returns the raw log time in internal format.
       */
      RawLogTime raw();
    
      /**
       * Calculates a time offset from the mostRecent log-entry timestamp (which usually stems from a 
       * log entry prior to board reset or power down).
       */
      void adjust(Timestamp mostRecent);
      
      /**
       * Resets the offset to 0 which means that log time and millis() coincide again in terms of seconds elapsed.
       */
      void reset();

    protected:
      
      /*
       * The value (in seconds) added to Arduino board millis().
       */
      uint32_t timeBase_sec = 0L;
      
      /*
       * The (adjusted) seconds of time when a the last timestamp was issued.
       */
      uint32_t last_sec = 0L;
      
      /*
       * The ID count of the last issued timestamp.
       */
      uint8_t timestampCount = 0;
  };


  #define MAX_TIMESTAMP_STR_LEN 13
  /**
   * Returns the timestamp in a 13-character dotted notation, terminated by '\0': sssssssss.cc  (s = seconds: 2^28 = 268435456 (9 digits), cc = count: 0..15 (2 digits))
   */
  char *formatTimestamp(Timestamp t, char buf[MAX_TIMESTAMP_STR_LEN]);
  
#endif
