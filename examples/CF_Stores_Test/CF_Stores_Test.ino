#include <ArduinoUnitX.h>

#define UNIT_TEST

#include <CF_Store.h>
#if not defined(ARDUINO_SAMD_ZERO) && not defined(ARDUINO_SAMD_MKR1000)
  #include <CF_EEPROM.h>
#endif
#include <CF_FRAM.h>

//#define DEBUG_UT_LOGGING

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  //Test::min_verbosity = TEST_VERBOSITY_ALL;
  //Test::exclude("*");
  //Test::include("*FRAM");
}

void loop() {
  Test::run();
}


struct Data {
  int8_t  id;
  int16_t p[2];
};

#define IDX_A 0
#define IDX_B sizeof(uint8_t)
#define IDX_C (IDX_B + sizeof(uint32_t))
#define STORE_SIZE (IDX_C + sizeof(Data))
#define STORE_OFFSET 40


// ------ Unit Tests --------

test(a_RAM) {
  RAMStore store = RAMStore(STORE_SIZE);
  assertFalse(store.expiringMedia());
  readWrite(&store, __FILE_NAME_STR);
}


test(b_EEPROM) {
  #if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_SAMD_MKR1000)
    Serial.println(F("SAMD M0 board does not feature EEPROM"));
    fail();
  #else
    EEPROMStore store = EEPROMStore(STORE_OFFSET, STORE_SIZE);
    assertTrue(store.expiringMedia());
    readWrite(&store, __FILE_NAME_STR)
  #endif
}


test(c_FRAM) {
  FRAMStore store1 = FRAMStore(STORE_OFFSET, STORE_SIZE);
  assertFalse(store1.expiringMedia());
  bool connected = store1.init();
  assertTrue(connected);
  readWrite(&store1, __FILE_NAME_STR);
}

test(d_FRAM) {
  FRAMStore store1 = FRAMStore(STORE_OFFSET, STORE_SIZE); 
  FRAMStore store2 = FRAMStore(&store1, STORE_SIZE);
  bool connected = store1.init();
  assertTrue(connected); 
  
  // check that store 1 can read and write:
  store1.write8(0, 101);
  uint8_t r1 = store1.read8(0);
  assertEqual(101, r1);
  
  // check that store 2 can read and write:
  store2.write8(0, 202);
  uint8_t r2 = store2.read8(0);
  assertEqual(202, r2);
  
  // check that there is no interference between reads and writes of the two stores:
  fill(&store1, 111);
  fill(&store2, 222);
  check(&store1, 111, __FILE_NAME_STR);
  check(&store2, 222, __FILE_NAME_STR);
}


void readWrite(AbstractStore *store, const __FlashStringHelper *__FILE_NAME_STR) {
  uint8_t  a = 1;
  uint32_t b = 2000;
  Data     c;
  c.id = 3;
  c.p[0] = 4000;
  c.p[1] = 5000;
  
  clear(store, __FILE_NAME_STR);
  
  //
  // 1 byte
  //
  store->write8(IDX_A, a);
  uint8_t ra = store->read8(IDX_A);
  assertEqual(a, ra);
  
  // update with same value => cell should *not* be updated
  bool updated = store->update8(IDX_A, a);
  assertFalse(updated);
  ra = store->read8(IDX_A);
  assertEqual(a, ra);

  // update with different value => cell should be updated
  a = 2;
  updated = store->update8(IDX_A, a);
  assertTrue(updated);
  ra = store->read8(IDX_A);
  assertEqual(a, ra);

  //
  // 4 bytes:
  //
  store->write(IDX_B, b);
  uint32_t rb = 0;
  store->read(IDX_B, rb);
  assertEqual(b, rb);

  // update with same value
  updated = store->update(IDX_B, b);
  assertFalse(updated);
  rb = 0;
  store->read(IDX_B, rb);
  assertEqual(b, rb);

  b = 2222;
  updated = store->update(IDX_B, b);
  assertTrue(updated);
  rb = 0;
  store->read(IDX_B, rb);
  assertEqual(b, rb);

  //
  // struct:
  //
  store->write(IDX_C, c);
  Data rc;
  store->read(IDX_C, rc);
  assertEqual(c.id, rc.id);
  assertEqual(c.p[0], rc.p[0]);
  assertEqual(c.p[1], rc.p[1]);
  
  // update with same value
  updated = store->update(IDX_C, c);
  assertFalse(updated);
  store->read(IDX_C, rc);
  assertEqual(c.id, rc.id);

  // update with different value
  c.id = 4;
  updated = store->update(IDX_C, c);
  assertTrue(updated);
  store->read(IDX_C, rc);
  assertEqual(c.id, rc.id);
  
  clear(store, __FILE_NAME_STR);
}


void clear(AbstractStore *store, const __FlashStringHelper *__FILE_NAME_STR) {
  store->clear();
  check(store, 0, __FILE_NAME_STR);
}

void fill(AbstractStore *store, const uint8_t val) {
  for (uint32_t i=0; i<STORE_SIZE; i++) {
    store->write8(i, val);
  }
}

void check(AbstractStore *store, const uint8_t val, const __FlashStringHelper *__FILE_NAME_STR) {
  for (uint32_t i=0; i<STORE_SIZE; i++) {
    uint8_t v = store->read8(i);
    assertEqual(v, val);
  }
}