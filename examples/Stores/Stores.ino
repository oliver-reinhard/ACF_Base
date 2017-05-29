#include <ACF_Store.h>
#if not defined(ARDUINO_SAMD_ZERO) && not defined(ARDUINO_SAMD_MKR1000)
  #include <ACF_EEPROM.h>
#endif
#include <ACF_FRAM.h>

struct Data {
  int8_t  id;
  int16_t p[2];
};

#define IDX_A 0
#define IDX_B sizeof(uint8_t)
#define IDX_C (IDX_B + sizeof(uint32_t))
#define STORE_SIZE (IDX_C + sizeof(Data))
#define STORE_OFFSET 40

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect.
  }

  demoRAM();
  demoEEPROM();
  demoFRAM();
}

void loop() {
  // empty
}


// ------ Unit Tests --------

void demoRAM() {
  Serial.println(F("---- RAM ----"));
  RAMStore store = RAMStore(STORE_SIZE);
  readWrite(&store);
}


void demoEEPROM() {
  Serial.println(F("---- EEPROM ----"));
  #if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_SAMD_MKR1000)
    Serial.println(F("SAMD M0 board does not feature EEPROM -> do nothing"));
  #else
    EEPROMStore store = EEPROMStore(STORE_OFFSET, STORE_SIZE);
    readWrite(&store);
  #endif
}


void demoFRAM() {
  Serial.println(F("---- FRAM ----"));
  Serial.println(F("NOTE: If you don’t have FRAM memory installed, then don’t worry about the following error messages"));
  FRAMStore store1 = FRAMStore(STORE_OFFSET, STORE_SIZE); 
  FRAMStore store2 = FRAMStore(&store1, STORE_SIZE);
  bool connected = store1.init();
  
  readWrite(&store1);
  
  // check that store 1 can read and write:
  store1.write8(0, 101);
  uint8_t r1 = store1.read8(0);
  
  // check that store 2 can read and write:
  store2.write8(0, 202);
  uint8_t r2 = store2.read8(0);
  
  // show that there is no interference between reads and writes of the two stores:
  fill(&store1, 111);
  fill(&store2, 222);
  read(&store1, 111);
  read(&store2, 222);
}


void readWrite(AbstractStore *store) {
  uint8_t  a = 1;
  uint32_t b = 2000;
  Data     c;
  c.id = 3;
  c.p[0] = 4000;
  c.p[1] = 5000;
  
  store->clear();
  
  //
  // 1 byte
  //
  Serial.println(F("  - write & read byte"));
  store->write8(IDX_A, a);
  uint8_t ra = store->read8(IDX_A);
  
  // update with same value => cell should *not* be updated
  bool updated = store->update8(IDX_A, a);
  ra = store->read8(IDX_A);

  // update with different value => cell should be updated
  a = 2;
  updated = store->update8(IDX_A, a);
  ra = store->read8(IDX_A);

  //
  // 4 bytes:
  //
  Serial.println(F("  - write & read int"));
  store->write(IDX_B, b);
  uint32_t rb = 0;
  store->read(IDX_B, rb);

  // update with same value
  updated = store->update(IDX_B, b);
  rb = 0;
  store->read(IDX_B, rb);

  b = 2222;
  updated = store->update(IDX_B, b);
  rb = 0;
  store->read(IDX_B, rb);

  //
  // struct:
  //
  Serial.println(F("  - write & read struct"));
  store->write(IDX_C, c);
  Data rc;
  store->read(IDX_C, rc);
  
  // update with same value
  updated = store->update(IDX_C, c);
  store->read(IDX_C, rc);

  // update with different value
  c.id = 4;
  updated = store->update(IDX_C, c);
  store->read(IDX_C, rc);
  
  store->clear();
}

void fill(AbstractStore *store, const uint8_t val) {
  for (uint32_t i=0; i<STORE_SIZE; i++) {
    store->write8(i, val);
  }
}

void read(AbstractStore *store, const uint8_t val) {
  for (uint32_t i=0; i<STORE_SIZE; i++) {
    uint8_t v = store->read8(i);
  }
}
