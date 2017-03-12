#include <CF_Logging.h>

// #define DEBUG_LOG

#define LOG_ENTRY_SIZE sizeof(LogEntry)
#define NUM_SLOTS_SIZE sizeof(uint16_t)


AbstractLog::AbstractLog(AbstractStore *store) {
  ASSERT(store != NULL, "constructor:store");
  this->store = store;
  logEntrySlots = (store->size() - NUM_SLOTS_SIZE) / LOG_ENTRY_SIZE;
}   

uint16_t AbstractLog::entryOffset(uint16_t index) {
  return NUM_SLOTS_SIZE + index * LOG_ENTRY_SIZE;
}

uint16_t AbstractLog::maxLogEntries() {
  return logEntrySlots - 1;
}

uint16_t AbstractLog::currentLogEntries() {
  if (logHeadIndex == logTailIndex) return 0;
  if (logHeadIndex > logTailIndex) return logHeadIndex - logTailIndex;
  return logEntrySlots - (logTailIndex - logHeadIndex);  // (logTailIndex - logHeadIndex) always differs by at least 1
}

void AbstractLog::clear() {
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: resetLog() [new] log size: "));
    Serial.println(logEntrySlots);
  #endif
  store->update(0, logEntrySlots);
  // clear
  for (uint16_t i = 0; i < logEntrySlots; i++) {
    clearLogEntry(i);
  }
  logTime.reset();
  logHeadIndex = 0;
  logTailIndex = 0;
  // write a log message so there is always at least one log entry:
  logMessage(MSG_LOG_INIT, 0, 0);
  lastNotifiedLogEntryIndex = logEntrySlots - 1; // = the one before the current entry at index 0
}


void AbstractLog::init() {
  //
  // Check if the number of log entries has changed (typically by changing from unit tests to production):
  //
  uint16_t oldMaxLogEntries;
  store->read(0, oldMaxLogEntries);
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: initlog() stored log size: "));
    Serial.println(oldMaxLogEntries);
  #endif
  if (oldMaxLogEntries != logEntrySlots) {
    clear();
    logMessage(MSG_LOG_SIZE_CHG, oldMaxLogEntries, logEntrySlots);
    return;
  }
  
  LogEntry entry;
  uint16_t mostRecentIndex = logEntrySlots;  // index of the most recent log entry (value is out of range => assert later that it was updated!)
  Timestamp mostRecentTimestamp = 0L;        // timestamp of the  most recent log entry
  logHeadIndex = logEntrySlots;              // value is out of range => assert later that logHeadIndex was updated!
  
  // find log head (= first empty log entry)
  for (uint16_t i = 0; i < logEntrySlots ; i++) {
    store->read(entryOffset(i), entry);
    
    if (entry.timestamp == 0L) {
      logHeadIndex = i;
      mostRecentIndex = (logEntrySlots + logHeadIndex - 1) % logEntrySlots;  // (logHeadIndex -1) can be negative => % function returns 0 ... !! => ensure always >= 0
      if(i == 0) {
        // if the head (= empty entry) is the very first entry of the array, then the most recent one is the very last one:
        LogEntry mostRecentEntry;
        store->read(entryOffset(mostRecentIndex), mostRecentEntry);      
        mostRecentTimestamp = mostRecentEntry.timestamp;
      }
      break;
    } 
    mostRecentTimestamp = entry.timestamp;
  }
  
  ASSERT(mostRecentIndex != logEntrySlots, "initLog:index");
  lastNotifiedLogEntryIndex = mostRecentIndex;
  
  ASSERT(logHeadIndex != logEntrySlots, "initLog:head");
  
  ASSERT(mostRecentTimestamp != 0L, "initLog:timestamp");
  logTime.adjust(mostRecentTimestamp);
  
  logTailIndex = logEntrySlots;    // value is out of range => assert later that logTailIndex was updated!
  for (uint16_t i = 1; i < logEntrySlots ; i++) {
    uint16_t index = (logHeadIndex + i) % logEntrySlots;
    store->read(entryOffset(index), entry);
    if (entry.timestamp != 0L) {
      logTailIndex = index;
      break;
    }
  }
  ASSERT(logTailIndex != logEntrySlots , "initLog:tail");
  #ifdef DEBUG_LOG
	Serial.print(F("           initlog() entries: "));
	Serial.println(currentLogEntries());
  #endif
}

void AbstractLog::clearLogEntry(uint16_t index) {
  reader.valid = false;
  uint16_t offset = entryOffset(index);
  for (uint16_t i = 0; i < LOG_ENTRY_SIZE ; i++) {
    store->update8(offset + i, 0);
  }
}


/*
 * Generic log-entry creation.
 */
LogEntry AbstractLog::addLogEntry(LogDataTypeID type, LogData *data) {
  LogEntry entry;
  entry.timestamp = logTime.timestamp();
  entry.type = type;
  memcpy(&(entry.data), data, sizeof(LogData));
  
  store->update(entryOffset(logHeadIndex), entry);
  logHeadIndex = (logHeadIndex + 1) % logEntrySlots;
  if (logHeadIndex == logTailIndex) {
    logTailIndex = (logTailIndex + 1) % logEntrySlots;
  }
  // clear the next entry
  clearLogEntry(logHeadIndex);
  #ifdef DEBUG_LOG
	Serial.print(F("DEBUG_LOG: addLogEntry() type: "));
	Serial.print(entry.type);
	Serial.print(F(" timestamp: "));
	char buf[MAX_TIMESTAMP_STR_LEN];
	Serial.print(formatTimestamp(entry.timestamp, buf));
	Serial.print(F(" head: "));
	Serial.print(logHeadIndex);
	Serial.print(F(" tail: "));
	Serial.println(logTailIndex);
  #endif
	
  return entry;
}

void AbstractLog::readMostRecentLogEntries(uint16_t maxResults) {
  reader.kind = LOG_READER_MOST_RECENT;
  uint16_t n = currentLogEntries();
  if (maxResults == 0) {
      reader.toRead = n;
  } else {
    reader.toRead = maxResults < n ? maxResults : n;
  }
  reader.read = 0;
  reader.nextIndex = (logEntrySlots + logHeadIndex - 1) % logEntrySlots; // (logHeadIndex -1) can be negative => % function returns 0 ... !! => ensure always >= 0
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: readMostRecentLogEntries() nextIndex: "));
    Serial.println(reader.nextIndex);
  #endif
  reader.valid = true;
}


void AbstractLog::readUnnotifiedLogEntries() {
  reader.kind = LOG_READER_UNNOTIFIED;
  if (logHeadIndex > lastNotifiedLogEntryIndex) {
    reader.toRead = logHeadIndex - lastNotifiedLogEntryIndex - 1;
  } else {
    reader.toRead = logEntrySlots - (lastNotifiedLogEntryIndex - logHeadIndex) - 1;
  }
  reader.read = 0;
  reader.nextIndex = (lastNotifiedLogEntryIndex + 1) % logEntrySlots;
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: readUnnotifiedLogEntries() nextIndex: "));
    Serial.println(reader.nextIndex);
  #endif
  reader.valid = true;
}

 
boolean AbstractLog::nextLogEntry(LogEntry &entry) {
  if (reader.valid && reader.read < reader.toRead) {
    store->read(entryOffset(reader.nextIndex), entry); 
    #ifdef DEBUG_LOG
      Serial.print(F("DEBUG_LOG: nextLogEntry() timestamp: "));
      char buf[MAX_TIMESTAMP_STR_LEN];
      Serial.print(formatTimestamp(entry.timestamp, buf));
      Serial.print(F(", type: "));
      Serial.println(entry.type);
    #endif
    reader.read++;
    if (reader.kind == LOG_READER_MOST_RECENT) {
      reader.nextIndex = (reader.nextIndex - 1) % logEntrySlots;
      
    } else if (reader.kind == LOG_READER_UNNOTIFIED) {
      lastNotifiedLogEntryIndex = reader.nextIndex;
      reader.nextIndex = (reader.nextIndex + 1) % logEntrySlots;
    }
    #ifdef DEBUG_LOG
      Serial.print(F("DEBUG_LOG: nextLogEntry() nextIndex: "));
      Serial.println(reader.nextIndex);
    #endif
    return true;
  }
  return false;
}

/*
 * Infinite loop, never ends.
 */
void blink_S_O_S() {
  int pulse = 300; // [ms]
  while(1) {
    // S.O.S. . . . – – – . . .
    pulse = (pulse + 200) % 400; // toggles between 100 and 300 ms
    delay(pulse);
    for(byte i=0; i<3; i++) {
      digitalWrite(SOS_LED_PIN, HIGH);
      delay(pulse);
      digitalWrite(SOS_LED_PIN, LOW);
      delay(pulse); 
    }      
  }
}

void AbstractLog::log_S_O_S(MessageID id, int16_t param1, int16_t param2) {
  Timestamp ts = logMessage(id, param1, param2);
  ts = ts; // prevents warning: unused variable
  #ifdef DEBUG_LOG
    char buf[MAX_TIMESTAMP_STR_LEN];
    Serial.print(F("DEBUG_LOG: S.O.S. See log message "));
    Serial.println(formatTimestamp(ts, buf));
  #endif
  blink_S_O_S();
}

void write_S_O_S(const __FlashStringHelper *msg, uint16_t line) {
  Serial.print(F("DEBUG_LOG: S.O.S. : "));
  Serial.print(msg);
  Serial.print(F(", line "));
  Serial.println(line);
  blink_S_O_S();
}


