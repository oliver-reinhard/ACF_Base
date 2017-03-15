#include <ACF_Configuration.h>

//#define DEBUG_CONFIG

/*
 * The magic number is the first byte of the config area. During startup it enables the detection of whether
 * the config area has been initialised before because the store cells of an (Arduino) board being read for the
 * very first time *cannot be assumed to be 0x0* !!
 */
const uint8_t MAGIC_NUMBER = 123;

#define MAGIC_NUMBER_SIZE sizeof(uint8_t)
#define VERSION_NUMBER_SIZE sizeof(uint8_t)
#define STORE_PARAM_OFFSET (MAGIC_NUMBER_SIZE + VERSION_NUMBER_SIZE)

#if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_SAMD_MKR1000)
  #define POINTER_SIZE sizeof(uint32_t)
#else
  #define POINTER_SIZE sizeof(uint16_t)
#endif

/*
 * Classes with virtual methods come with a "superclass" pointer occupying the
 * first two bytes of the in-memory class object:
 */
#define SUPERCLASS_PTR_SIZE POINTER_SIZE
/*
 * Pointer to AbstractStore object:
 */
#define STORE_PTR_SIZE POINTER_SIZE
/*
 * This is where the actual configuration params starts in RAM (byte offset):
 */
#define RAM_PARAM_OFFSET (SUPERCLASS_PTR_SIZE + STORE_PTR_SIZE + VERSION_NUMBER_SIZE)


AbstractConfigParams::AbstractConfigParams(AbstractStore *store, const uint8_t version) {
  this->store = store;
  this->layoutVersion = version;
}

uint8_t AbstractConfigParams::magicNumber() {
  return store->read8(0);
}

uint8_t AbstractConfigParams::version() {
  return store->read8(MAGIC_NUMBER_SIZE);
}

void AbstractConfigParams::clear() {
  #ifdef DEBUG_CONFIG
    Serial.println(F("DEBUG_CONFIG Clear"));
  #endif
  store->clear();
  // copy 0x0 values from store to memory:
  readParams();
}


void AbstractConfigParams::load() {
  #ifdef DEBUG_CONFIG
   Serial.println(F("DEBUG_CONFIG Load"));
  #endif
  
  if (magicNumber() != MAGIC_NUMBER) {
    #ifdef DEBUG_CONFIG
      Serial.println(F("DEBUG_CONFIG Initialising store (first use)"));
    #endif
    clear(); // invokes readParams()
    store->write8(0, MAGIC_NUMBER);
    store->write8(MAGIC_NUMBER_SIZE, layoutVersion);
    
  } else if (version() != layoutVersion) {
    #ifdef DEBUG_CONFIG
      Serial.println(F("DEBUG_CONFIG Updating store version"));
    #endif
	store->write8(MAGIC_NUMBER_SIZE, layoutVersion);
    readParams();

  } else {
    readParams();
  }
  
  boolean updated;
  initParams(updated);
  if (updated) {
    #ifdef DEBUG_CONFIG
      Serial.println(F("DEBUG_CONFIG Applied (some) config value defaults"));
    #endif
    save();
  }
}

void AbstractConfigParams::save() {
  #ifdef DEBUG_CONFIG
    Serial.println(F("DEBUG_CONFIG Save"));
  #endif
  // Don't write magic number and version / layoutVersion:
  const uint8_t *ptr = (const uint8_t *) (this);
  ptr += RAM_PARAM_OFFSET;
  const uint32_t len = memSize() - RAM_PARAM_OFFSET;
  for (uint32_t i=0; i<len; i++) {
    store->update8(STORE_PARAM_OFFSET + i, *ptr++);
  }
}

void AbstractConfigParams::readParams() {
  #ifdef DEBUG_CONFIG
    Serial.println(F("DEBUG_CONFIG Read"));
  #endif
  // Don't read magic number and version / layoutVersion:
  uint8_t *ptr = (uint8_t *) (this);
  ptr += RAM_PARAM_OFFSET;
  const uint32_t len = memSize() - RAM_PARAM_OFFSET;
  for (uint32_t i=0; i<len; i++) {
	*ptr++ = store->read8(STORE_PARAM_OFFSET + i);
  }
}

void AbstractConfigParams::print() {
  Serial.print(F("Version: "));
  Serial.println(version());
}
