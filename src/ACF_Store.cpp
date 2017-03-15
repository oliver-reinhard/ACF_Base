
#include <ACF_Store.h>
  
//#define DEBUG_STORE
  
void RAMStore::clear() {
  const uint32_t len = offsetBytes + sizeBytes;
  #ifdef DEBUG_STORE
	Serial.print(F("DEBUG_STORE *Clear [0.."));
	Serial.print(len-1);
	Serial.println(']');
  #endif
  for (uint32_t i = offsetBytes;  i < len ; i++) {
    memory[i] = 0x0;
  }
}

uint8_t RAMStore::read8(uint32_t idx) {
  const uint8_t val = memory[idx];
  #if defined DEBUG_STORE
    Serial.print(F("DEBUG_STORE read  ["));
    Serial.print(idx);
    Serial.print(F("] -> 0x"));
    Serial.println(val, HEX);
  #endif
  return val;
}

void RAMStore::write8(uint32_t idx, uint8_t val) {
  memory[idx] = val;
  #ifdef DEBUG_STORE
    Serial.print(F("DEBUG_STORE write ["));
    Serial.print(idx);
    Serial.print(F("] := 0x"));
    Serial.println(val, HEX);
  #endif
}

bool RAMStore::update8(uint32_t idx, uint8_t val) {
  if (memory[idx] == val) return false;
  memory[idx] = val;
  #ifdef DEBUG_STORE
    Serial.print(F("DEBUG_STORE upd   ["));
    Serial.print(idx);
    Serial.print(F("] := 0x"));
    Serial.println(val, HEX);
  #endif
  return true;
}

