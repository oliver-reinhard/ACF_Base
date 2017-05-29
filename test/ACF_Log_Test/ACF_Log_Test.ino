#include <ArduinoUnit.h>

#define UNIT_TEST
#include <ACF_Messages.h>
#include <ACF_Store.h>
#include <ACF_Logging.h>

//#define DEBUG_UT_LOGGING

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  //Test::min_verbosity = TEST_VERBOSITY_ALL;
  //Test::exclude("*");
  //Test::include("c_log_init_clear*");
  //Test::exclude("z_s_o_s");
}

void loop() {
  Test::run();
}


const uint16_t UNIT_TEST_LOG_ENTRIES = 5;  // numer of LogEntry slots
const uint16_t STORE_SIZE = sizeof(uint8_t) + sizeof(uint16_t) + UNIT_TEST_LOG_ENTRIES * sizeof(LogEntry);  // uint16_t = number of logEntries

enum class LogDataType : T_LogDataType_ID {
  MESSAGE = 0,
  VALUES = 1
};

struct LogMessageData {
  T_Message_ID id;
  int16_t   params[2];
};

static_assert(sizeof(LogMessageData) <= sizeof(LogData), "LogMessageData > LogData");

struct LogValuesData {
  int16_t value;
  byte    filler[3];
};

static_assert(sizeof(LogValuesData) <= sizeof(LogData), "LogValuesData > LogData");


class TestLog : public AbstractLog {
  public:
    TestLog(AbstractStore *store) : AbstractLog(store) { }; 
  
    Timestamp logMessage(T_Message_ID id, int16_t param1, int16_t param2) {
      LogMessageData data;
      memset(&data, 0x0, sizeof(data));
      data.id = id;
      data.params[0] = param1;
      data.params[1] = param2;
      LogEntry e = addLogEntry(static_cast<T_LogDataType_ID>(LogDataType::MESSAGE), (LogData *) &data);
      return e.timestamp;
    }

    Timestamp logValues(int16_t value) {
      LogValuesData data;
      memset(&data, 0x0, sizeof(data));
      data.value = value;
      LogEntry e = addLogEntry(static_cast<T_LogDataType_ID>(LogDataType::VALUES), (LogData *) &data);
      return e.timestamp;
    }
};

// ------ Unit Tests --------

test(a_log_ring_buffer) {
  RAMStore store = RAMStore(STORE_SIZE); 
  TestLog logging = TestLog(&store);

  logging.clear();
  assertEqual(logging.maxLogEntries(), UNIT_TEST_LOG_ENTRIES - 1);
  assertEqual(logging.currentLogEntries(), 1u);
  assertEqual(logging.logTailIndex, 0u);
  assertEqual(logging.logHeadIndex, 1u);

  // Test ring buffer logging:
  logging.logValues(3000);
  assertEqual(logging.currentLogEntries(), 2u);
  assertEqual(logging.logTailIndex, 0u);
  assertEqual(logging.logHeadIndex, 2u);
  
  logging.logValues(3100);
  assertEqual(logging.currentLogEntries(), 3u);
  assertEqual(logging.logTailIndex, 0u);
  assertEqual(logging.logHeadIndex, 3u);
  
  logging.logValues(3200);
  assertEqual(logging.currentLogEntries(), 4u);
  assertEqual(logging.logTailIndex, 0u);
  assertEqual(logging.logHeadIndex, 4u);
  
  logging.logValues(3300);
  assertEqual(logging.currentLogEntries(), 4u);
  assertEqual(logging.logTailIndex, 1u);
  assertEqual(logging.logHeadIndex, 0u);
  
  logging.logValues(3400);
  assertEqual(logging.currentLogEntries(), 4u);
  assertEqual(logging.logTailIndex, 2u);
  assertEqual(logging.logHeadIndex, 1u);
  
  logging.logValues(3500);
  assertEqual(logging.currentLogEntries(), 4u);
  assertEqual(logging.logTailIndex, 3u);
  assertEqual(logging.logHeadIndex, 2u);
}

test(b_log_init) {
  RAMStore store = RAMStore(STORE_SIZE); 
  TestLog logging = TestLog(&store);

  // Test initialisation:
  logging.clear();
  logging.init();
  assertEqual(logging.logTailIndex, 0u);
  assertEqual(logging.logHeadIndex, 1u);
  
  logging.logValues(3000);
  logging.init();
  assertEqual(logging.logTailIndex, 0u);
  assertEqual(logging.logHeadIndex, 2u);
  
  logging.logValues(3100);
  logging.init();
  assertEqual(logging.logTailIndex, 0u);
  assertEqual(logging.logHeadIndex, 3u);
  
  logging.logValues(3200);
  logging.init();
  assertEqual(logging.logTailIndex, 0u);
  assertEqual(logging.logHeadIndex, 4u);
  
  logging.logValues(3300);
  logging.init();
  assertEqual(logging.logTailIndex, 1u);
  assertEqual(logging.logHeadIndex, 0u);
  
  logging.logValues(3400);
  logging.init();
  assertEqual(logging.logTailIndex, 2u);
  assertEqual(logging.logHeadIndex, 1u);
  
  logging.logValues(3500);
  logging.init();
  assertEqual(logging.logTailIndex, 3u);
  assertEqual(logging.logHeadIndex, 2u);
}

test(c_log_init_clear) {
  RAMStore store = RAMStore(STORE_SIZE); 
  TestLog logging = TestLog(&store);

  // Test initialisation:
  logging.clear();
  uint8_t magicNumber = store.read8(0);
  logging.init();
  logging.logValues(3000);
  logging.logValues(3100);
  assertEqual(logging.currentLogEntries(), 3u);

  // delete magic number --> must clear and rewrite magic number
  store.write8(0, 0);
  assertEqual(0, store.read8(0));
  logging.init();
  assertEqual(magicNumber, store.read8(0));
  assertEqual(logging.currentLogEntries(), 2u);
}

test(d_log_reader_unnotified) {
  RAMStore store = RAMStore(STORE_SIZE); 
  TestLog logging = TestLog(&store);

  logging.clear(); // => creates a first log entry
  logging.logValues(3000);
  logging.logValues(3100);
  logging.logValues(3200);
  
  logging.readUnnotifiedLogEntries(); // returns oldest first, then --> newer
  assertEqual(logging.reader.toRead, 4u);
  LogEntry e;
  //
  assertTrue(logging.nextLogEntry(e));
  assertEqual(int(e.type), int(static_cast<T_LogDataType_ID>(LogDataType::MESSAGE)));
  LogMessageData lmd;
  memcpy(&lmd, &(e.data), sizeof(LogMessageData));
  assertEqual(lmd.id, static_cast<T_Message_ID>(ACF_Msg::LOG_INIT));
  //
  assertTrue(logging.nextLogEntry(e));
  assertEqual(int(e.type), int(static_cast<T_LogDataType_ID>(LogDataType::VALUES)));
  LogValuesData lvd1;
  memcpy(&lvd1, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd1.value, 3000);
  //
  // Stop with current reader, create a new reader
  //
  logging.readUnnotifiedLogEntries(); // returns oldest first, then --> newer
  assertEqual(logging.reader.toRead, 2u);
  //
  assertTrue(logging.nextLogEntry(e));
  assertEqual(int(e.type), int(static_cast<T_LogDataType_ID>(LogDataType::VALUES)));
  LogValuesData lvd2;
  memcpy(&lvd2, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd2.value, 3100);
  //
  assertTrue(logging.nextLogEntry(e));
  assertEqual(int(e.type), int(static_cast<T_LogDataType_ID>(LogDataType::VALUES)));
  LogValuesData lvd3;
  memcpy(&lvd3, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd3.value, 3200);
}

test(e_log_reader_most_recent) {
  RAMStore store = RAMStore(STORE_SIZE); 
  TestLog logging = TestLog(&store);

  logging.clear(); // => creates a first log entry
  logging.logValues(3000);
  logging.logValues(3100);
  logging.logValues(3200);
  
  logging.readMostRecentLogEntries(0); // returns most recent first, then --> older
  assertEqual(logging.reader.toRead, 4u);
  LogEntry e;
  //
  assertTrue(logging.nextLogEntry(e));
  assertEqual(int(e.type), int(static_cast<T_LogDataType_ID>(LogDataType::VALUES)));
  LogValuesData lvd1;
  memcpy(&lvd1, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd1.value, 3200);
  //
  assertTrue(logging.nextLogEntry(e));
  assertEqual(int(e.type), int(static_cast<T_LogDataType_ID>(LogDataType::VALUES)));
  LogValuesData lvd2;
  memcpy(&lvd2, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd2.value, 3100);
  //
  assertTrue(logging.nextLogEntry(e));
  assertEqual(int(e.type), int(static_cast<T_LogDataType_ID>(LogDataType::VALUES)));
  LogValuesData lvd3;
  memcpy(&lvd3, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd3.value, 3000);
  //
  assertTrue(logging.nextLogEntry(e));
  assertEqual(int(e.type), int(static_cast<T_LogDataType_ID>(LogDataType::MESSAGE)));
  LogMessageData lmd;
  memcpy(&lmd, &(e.data), sizeof(LogMessageData));
  assertEqual(lmd.id, static_cast<T_Message_ID>(ACF_Msg::LOG_INIT));
  //
  assertFalse(logging.nextLogEntry(e));

  //
  // Test reading past the wrapping point:
  logging.logValues(3300);
  logging.logValues(3400);
  
  logging.readMostRecentLogEntries(0); // returns most recent first, then --> older
  assertEqual(logging.reader.toRead, 4u);
  
  assertTrue(logging.nextLogEntry(e));
  LogValuesData lvd4;
  memcpy(&lvd4, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd4.value, 3400);
  
  assertTrue(logging.nextLogEntry(e));
  LogValuesData lvd5;
  memcpy(&lvd5, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd5.value, 3300);
  
  assertTrue(logging.nextLogEntry(e));
  LogValuesData lvd6;
  memcpy(&lvd6, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd6.value, 3200);
  
  assertTrue(logging.nextLogEntry(e));
  LogValuesData lvd7;
  memcpy(&lvd7, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd7.value, 3100);
  
  assertFalse(logging.nextLogEntry(e));
}

test(z_s_o_s) {
  S_O_S(F("Program execution halted, S.O.S. Verify line number with test-code"));
}