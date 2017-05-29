#include <ACF_Store.h>
#include <ACF_Configuration.h>

#define CONFIG_SIZE_WITH_RESERVE  10  // includes a reserve of 4 bytes with respect to original param size of 6 bytes
#define CONFIG_VERSION_A          22
#define CONFIG_SIZE_A              6

// Param 1 is a 4-byte array:
#define PARAM_1_LENGTH             4
#define PARAM_1_DEFAULT_VALUE    111
#define PARAM_1_NEW_VALUE        222

/* This configuration block consists of one parameter only which is an array with 4 elements */
class TestConfig_A : public AbstractConfigParams {
  public:
    TestConfig_A(AbstractStore *store, const uint8_t version) : AbstractConfigParams(store, version)  { };

    // Actual configuration parameters
    uint8_t param1[PARAM_1_LENGTH];

    uint16_t memSize() { return sizeof(*this); };
    
    void initParams(boolean &updated) {
      updated = false;
      for(uint16_t i = 0; i < PARAM_1_LENGTH; i++) {
        if (param1[i] == 0) {
          param1[i] = PARAM_1_DEFAULT_VALUE + i;
          updated = true;
        }
      }
    }

    void print() {
      AbstractConfigParams::print();
      Serial.print(F("param1: {"));
      for(uint16_t i = 0; i < PARAM_1_LENGTH; i++) {
        Serial.print(param1[i]);
        Serial.print(' ');
      }
      Serial.println('}');
    }
};


void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  
  // We're using a RAM-based store so we needn't worry whether our board has EEPROM or not
  RAMStore store = RAMStore(CONFIG_SIZE_WITH_RESERVE);

  TestConfig_A config1 = TestConfig_A(&store, CONFIG_VERSION_A);

  // clear the area and all its stuctures (even RAM is not guaranteed to be zero-initialised).
  // This also wipes the magic number.
  config1.clear();
  config1.print();

  // load will initialize with default values if area is missing the magic number.
  config1.load();
  Serial.println("----------");
  config1.print();

  // modify a parameter value and save it.
  uint16_t index = PARAM_1_LENGTH-1;  
  config1.param1[index] = PARAM_1_NEW_VALUE;
  config1.save();

  // Reload into a different structure to prove that the values are loaded from the store:
  TestConfig_A config2 = TestConfig_A(&store, CONFIG_VERSION_A);
  config2.load();
  Serial.println("----------");
  config1.print();
}

void loop() {
  // empty
}
