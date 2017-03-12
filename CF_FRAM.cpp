
#include <CF_FRAM.h>

//#define DEBUG_FRAM


bool FRAMStore::init(uint8_t addr) {
	return fram->begin(addr);
}

void FRAMStore::clear() {
  const uint32_t maxIndex = offsetBytes + sizeBytes;
  #ifdef DEBUG_FRAM
	Serial.print(F("DEBUG_FRAM *Clear["));
	Serial.print(offsetBytes);
 	Serial.print("..");
	Serial.print(maxIndex-1);
	Serial.println(']');
  #endif
  for (uint32_t i = offsetBytes;  i < maxIndex ; i++) {
    fram->write8(i, 0x0);
  }
}

uint8_t FRAMStore::read8(uint32_t idx) {
  const uint8_t val = fram->read8(offsetBytes + idx);
  #if defined DEBUG_FRAM
    Serial.print(F("DEBUG_FRAM read  ["));
    Serial.print(offsetBytes + idx);
    Serial.print(F("] -> 0x"));
    Serial.println(val, HEX);
  #endif
  return val;
}

void FRAMStore::write8(uint32_t idx, uint8_t val) {
  fram->write8(offsetBytes + idx, val);
  #ifdef DEBUG_FRAM
    Serial.print(F("DEBUG_FRAM write ["));
    Serial.print(offsetBytes + idx);
    Serial.print(F("] := 0x"));
    Serial.println(val, HEX);
  #endif
}

bool FRAMStore::update8(uint32_t idx, uint8_t val) {
  const uint8_t current = fram->read8(offsetBytes + idx);
  if (current == val) return false;
  fram->write8(offsetBytes + idx, val);
  #ifdef DEBUG_FRAM
    Serial.print(F("DEBUG_FRAM upd   ["));
    Serial.print(offsetBytes + idx);
    Serial.print(F("] := 0x"));
    Serial.println(val, HEX);
  #endif
  return true;
}

