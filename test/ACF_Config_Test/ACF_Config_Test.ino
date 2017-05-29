
#include <ArduinoUnit.h>

#define UNIT_TEST

#include <ACF_Store.h>
#include <ACF_Configuration.h>

#define DEBUG_UT_CONFIG

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  //Test::min_verbosity = TEST_VERBOSITY_ALL;
  //Test::exclude("*");
  //Test::include("params_a");
}

void loop() {
  Test::run();
}

// ------ Unit Tests --------

#define CONFIG_SIZE_WITH_RESERVE  10  // includes a reserve of 4 bytes with respect to original param size of 6 bytes
#define CONFIG_VERSION_A          22
#define CONFIG_SIZE_A              6
#define CONFIG_VERSION_B          23
#define CONFIG_SIZE_B              8
// Param 1 is a 4-byte array:
#define PARAM_1_LENGTH             4
#define PARAM_1_DEFAULT_VALUE    111
#define PARAM_1_NEW_VALUE        222
// Param 2 is a int16_t:
#define PARAM_2_DEFAULT_VALUE    333
#define PARAM_2_NEW_VALUE        444

class TestConfig_A : public AbstractConfigParams {
  public:
    TestConfig_A(AbstractStore *store, const uint8_t version) : AbstractConfigParams(store, version)  { };

    // Actual configuration parameters (are public for test verification purposes)
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

class TestConfig_B : public TestConfig_A {
  public:
    TestConfig_B(AbstractStore *store, const uint8_t version) : TestConfig_A(store, version)  { };

    // Actual configuration parameters (are public for test verification purposes)
    int16_t param2;

    uint16_t memSize() { return sizeof(*this); };
    
    void initParams(boolean &updated) {
      TestConfig_A::initParams(updated);
      updated = false;
      if (param2 == 0) {
          param2 = PARAM_2_DEFAULT_VALUE;
          updated = true;
      }
    }

    void print() {
      TestConfig_A::print();
        Serial.print(F("param2: "));
        Serial.println(param2);
    }
};

test(params_a) {
  RAMStore store = RAMStore(CONFIG_SIZE_WITH_RESERVE);
  TestConfig_A config1 = TestConfig_A(&store, CONFIG_VERSION_A);
  assertEqual(config1.memSize(), sizeof(TestConfig_A));
  
  config1.clear();
  assertEqual(config1.version(), 0);
 
  config1.load();
  #ifdef DEBUG_UT_CONFIG
    config1.print();
  #endif
  assertEqual(config1.version(), CONFIG_VERSION_A);

  assertEqual(config1.param1[0], PARAM_1_DEFAULT_VALUE); 
  uint16_t index = PARAM_1_LENGTH-1;
  assertEqual(config1.param1[index], PARAM_1_DEFAULT_VALUE + index); 
  
  config1.param1[index] = PARAM_1_NEW_VALUE;
  config1.save();

  // Reload:
  TestConfig_A config2 = TestConfig_A(&store, CONFIG_VERSION_A);
  config2.load();
  assertEqual(config2.version(), CONFIG_VERSION_A);
  assertEqual(config2.param1[0], PARAM_1_DEFAULT_VALUE);
  assertEqual(config2.param1[index], PARAM_1_NEW_VALUE);
  
  config2.clear();
  assertEqual(config2.version(), 0L);
  assertEqual(config2.param1[0], 0);
  assertEqual(config2.param1[index], 0);
}

test(params_b) {
  RAMStore store = RAMStore(CONFIG_SIZE_WITH_RESERVE);
  TestConfig_A configA = TestConfig_A(&store, CONFIG_VERSION_A);
  configA.load();
  assertEqual(configA.version(), CONFIG_VERSION_A);
  uint16_t index = PARAM_1_LENGTH-1;
  configA.param1[index] = PARAM_1_NEW_VALUE;
  configA.save();

  //
  // Extend Config A by another parameter and check transition to the new version:
  //
  TestConfig_B configB = TestConfig_B(&store, CONFIG_VERSION_B);
  assertEqual(configB.memSize(), sizeof(TestConfig_B));
 
  configB.load();
  #ifdef DEBUG_UT_CONFIG
    configB.print();
  #endif
  assertEqual(configB.version(), CONFIG_VERSION_B);
  assertEqual(configB.param1[index], PARAM_1_NEW_VALUE);

  assertEqual(configB.param2, PARAM_2_DEFAULT_VALUE);
  configB.param2 = PARAM_2_NEW_VALUE;
  configB.save();
  
  // Reload:
  TestConfig_B configB2 = TestConfig_B(&store, CONFIG_VERSION_B);
  configB2.load();
  #ifdef DEBUG_UT_CONFIG
    configB2.print();
  #endif
  assertEqual(configB2.version(), CONFIG_VERSION_B);
  assertEqual(configB2.param2, PARAM_2_NEW_VALUE);
}
