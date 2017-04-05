#ifndef ACF_TYPES_H_INCLUDED
  #define ACF_TYPES_H_INCLUDED
  
  #include <Arduino.h>

  typedef uint32_t TimeMillis;
  typedef int32_t TimeSeconds;

  static const TimeSeconds UNDEFINED_TIME_SECONDS = -1L;

  /* Storage type for message identifiers.  */
  typedef uint16_t T_Message_ID;

  /* Storage type for message parameters. */
  typedef int16_t T_Message_Param;

#endif
