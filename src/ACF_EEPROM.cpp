// AMD M0 processors don't have an EEPROM:
#if not defined(ARDUINO_SAMD_ZERO) && not defined(ARDUINO_SAMD_MKR1000)

#include <ACF_EEPROM.h>
  
//#define DEBUG_EEPROM
  
void EEPROMStore::clear() {
  const uint32_t maxIndex = offsetBytes + sizeBytes;
  #ifdef DEBUG_EEPROM
	Serial.print(F("DEBUG_EEPROM *Clear["));
	Serial.print(offsetBytes);
 	Serial.print("..");
	Serial.print(maxIndex-1);
	Serial.println(']');
  #endif
  for (uint32_t i = offsetBytes;  i < maxIndex ; i++) {
    EEPROM.write(i, 0x0);
  }
}

uint8_t EEPROMStore::read8(uint32_t idx) {
  const uint8_t val = EEPROM.read(offsetBytes + idx);
  #if defined DEBUG_EEPROM
    Serial.print(F("DEBUG_EEPROM read  ["));
    Serial.print(offsetBytes + idx);
    Serial.print(F("] -> 0x"));
	Serial.println(val, HEX);
  #endif
  return val;
}

void EEPROMStore::write8(uint32_t idx, uint8_t val) {
  EEPROM.write(offsetBytes + idx, val);
  #ifdef DEBUG_EEPROM
    Serial.print(F("DEBUG_EEPROM write ["));
    Serial.print(offsetBytes + idx);
    Serial.print(F("] := 0x"));
    Serial.println(val, HEX);
  #endif
}

bool EEPROMStore::update8(uint32_t idx, uint8_t val) {
  const uint8_t current = EEPROM.read(offsetBytes + idx);
  if (current == val) return false;
  EEPROM.write(offsetBytes + idx, val);
  #ifdef DEBUG_EEPROM
    Serial.print(F("DEBUG_EEPROM upd   ["));
    Serial.print(offsetBytes + idx);
    Serial.print(F("] := 0x"));
    Serial.println(val, HEX);
  #endif
  return true;
}

#endif
