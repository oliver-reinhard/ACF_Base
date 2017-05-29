#include <ACF_Messages.h>
#include <ACF_Store.h>
#include <ACF_Logging.h>

const uint16_t DEMO_LOG_ENTRIES = 5;  // numer of LogEntry slots
const uint16_t STORE_SIZE = sizeof(uint8_t) + sizeof(uint16_t) + DEMO_LOG_ENTRIES * sizeof(LogEntry);  // uint8_t => magic number; uint16_t => number of logEntries

// define types of log entries:
enum class LogDataType : T_LogDataType_ID {
  MESSAGE = 0,
  VALUES = 1
};

// define message entries:
struct LogMessageData {
  T_Message_ID id;
  int16_t   params[2];
};

// ensure the generic LogData structure is big enough to hold log messages as defined above:
static_assert(sizeof(LogMessageData) <= sizeof(LogData), "LogMessageData > LogData");

// define value-log entries:
struct LogValuesData {
  int16_t value;
  byte    filler[3];
};

// ensure the generic LogData structure is big enough to hold logged values as defined above:
static_assert(sizeof(LogValuesData) <= sizeof(LogData), "LogValuesData > LogData");


class MyLog : public AbstractLog {
  public:
    MyLog(AbstractStore *store) : AbstractLog(store) { }; 
  
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

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  
  // We're using a RAM-based store so we needn't worry whether our board has EEPROM or not
  RAMStore store = RAMStore(STORE_SIZE); 

  demo1(&store);
  demo2(&store);
  demo3(&store);
}

void loop() {
  // empty
}

void demo1(RAMStore *store) {
  MyLog logging = MyLog(store);

  logging.clear(); // => implicitly creates a first log entry
  logging.logValues(3000);
  logging.logValues(3100);
  logging.readUnnotifiedLogEntries(); // returns oldest first, then --> newer
  listEntries(&logging, 1);
  
  logging.logValues(3200);
  logging.readUnnotifiedLogEntries(); // returns only 1 entry
  listEntries(&logging, 2);
}


void demo2(RAMStore *store) {
  // initialize a new logging on existing store:
  MyLog logging = MyLog(store);
  logging.init();

  // list all entries (=> 0)
  logging.readMostRecentLogEntries(0); // returns most recent first, then --> older
  listEntries(&logging, 3);
  
  // list 2 most recent entries (=> 2)
  logging.readMostRecentLogEntries(2); // returns most recent first, then --> older
  listEntries(&logging, 4);
}

void demo3(RAMStore *store) {
  // initialize a new logging on existing store:
  MyLog logging = MyLog(store);
  logging.init();
  
  // list all entries (=> 0) 
  logging.readMostRecentLogEntries(0); // returns most recent first, then --> older
  listEntries(&logging, 5);

  // add more entries and exceed log capacity => log will roll around and overwrite oldest entries:
  logging.logValues(3300);
  logging.logValues(3400);
  logging.logValues(3500);
  
  // list all entries (=> 0) 
  // Note: remember that one slot of the log always remains unoccupied, thus only expect (DEMO_LOG_ENTRIES - 1) entries.
  logging.readMostRecentLogEntries(0); // returns most recent first, then --> older
  listEntries(&logging, 6);
}


void listEntries(MyLog *logging, int id) {
  Serial.print("-------");
  Serial.print(id);
  Serial.println("-------");
  LogEntry e;
  while (logging->nextLogEntry(e)) {
    printLogEntry(e);
  }
}

void printLogEntry(LogEntry e) {
  if (e.type == int(static_cast<T_LogDataType_ID>(LogDataType::MESSAGE))) {
    LogMessageData lmd;
    memcpy(&lmd, &(e.data), sizeof(LogMessageData));
    Serial.print("Entry is a Message; id = ");
    Serial.println(lmd.id);

  } else if (e.type == int(static_cast<T_LogDataType_ID>(LogDataType::VALUES))) {
    LogValuesData lvd;
    memcpy(&lvd, &(e.data), sizeof(LogValuesData));
    Serial.print("Entry is a Value; value = ");
    Serial.println(lvd.value);
    
  } else {
    Serial.print("Unknown entry type: ");
    Serial.println(e.type);
  }
}
