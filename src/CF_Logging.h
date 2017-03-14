#ifndef CF_LOGGING_H_INCLUDED
  #define CF_LOGGING_H_INCLUDED
  
  #include <CF_LogTime.h>
  #include <CF_Store.h>

  // Define this symbol in an including module (prior to #include "CF_Logging.h") to have a different payload size (in Byte):
  //
  // Note: Memory alignment can mean that struct fields of 1 byte actually occupy 2, e.g. uint8_t
  #ifndef LOG_DATA_PAYLOAD_SIZE
    #define LOG_DATA_PAYLOAD_SIZE 6
  #endif

  // Define this symbol in an including module (prior to #include "CF_Logging.h") to define the LED pin for issuing fatal S.O.S.:
  #ifndef SOS_LED_PIN
    #define SOS_LED_PIN LED_BUILTIN
  #endif

  #define S_O_S(flashStringHelper) write_S_O_S(flashStringHelper, __LINE__)
  #define ASSERT(cond, msg) ((cond) ? (void)0 : S_O_S(F(msg)))

  /*
   *  Storage type for message identifiers.
   */
  typedef uint8_t MessageID;
  
  /*
   * The messages issued by the implementation of this module. Stored as MessageID.
   * Note: the actual message texts and the conversion of message IDs to text have to be implemented by 
   *       the consumer of this libraray 
   */
  typedef enum {
    MSG_SYSTEM_INIT = 0,      // The board is initialising from reset or power on [no parameters]
    MSG_LOG_INIT = 1,         // Log initialised [no parameters]
    MSG_LOG_MAGIC_NUMBER = 2, // Log was cleared because magic number was not detected ->
    MSG_LOG_SIZE_CHG = 3,     // Log was cleared because number of log entries has changed from [old] to [new])
  } AbstractLogMessageEnum;
  
  /**
   * Generic "supertype" for log data; "subtypes" are distinguished via LogDataTypeID.
   * Note: The actual payload size can be configured / changed via symbol definition (LOG_DATA_PAYLOAD_SIZE).
   */
  struct LogData {
    uint8_t payload[LOG_DATA_PAYLOAD_SIZE]; // placeholder
  };
  
  /*
   * Discriminator for various types of log data.
   */
  typedef uint8_t LogDataTypeID;
  
  /**
   * Actual log record. At runtime the data field is an instance of a "subtype" of LogData.
   */
  struct LogEntry {
    Timestamp timestamp;
    LogDataTypeID type;
    LogData   data; // generic
  };

    
  typedef enum {
    LOG_READER_MOST_RECENT = 0,  // reads newer to older
    LOG_READER_UNNOTIFIED = 1    // reads older to newer
  } LogReaderKindEnum;


  /*
   * Simple, non-concurrent reader for log entries.
   */
  struct LogReader {
    /*
     * Determines reader behaviour.
     */
    LogReaderKindEnum kind;
    /*
     * The values in this struct are defined only if valid == true.
     */
    boolean valid;
    /*
     * Number of entries to be returned through this reader (remains constant).
     */
    uint16_t toRead;
    /*
     * Number of entries alreday returned by this reader (increases with each entry read);
     */
    uint16_t read = 0;
    /*
     * Index of next entry that will be returned.
     */
    uint16_t nextIndex;
  };
  

  /*
   * Logging is done to a CF_Store::AbstractStore.
   * 
   * The log structure is as follows:
   * 
   * - logEntrySlots (=total number of log entry slots; used to detect changes => reset)
   * - Actual log entries (LogEntry[logEntrySlots])
   */
  class AbstractLog {
    
    public:
      
      /*
       * @param store physical store to use for persistent storage; cannot be null.
       */
      AbstractLog(AbstractStore *store);
      
      /**
       * Initialise in-memory log-managment structures from the log entries found in the EEPROM.
       * This is typically performed after an Arduino board-reset.
       * If the maximum number of log entries is found to be different from the previous run, then the
       * log is cleared and a message is logged to record the change in size.
       */
      virtual void init();
      
      /**
       * Clear all log entries on the store and reset in-memory log-managment structures.
       */
      virtual void clear();

      /*
       * The timestamp generator for this log.
       */
      LogTime logTime = LogTime();
      
      /*
       * Returns number of available slots for log entries.
       * Note: this is always 1 less than the actual number of slots because the next available slot is always cleared ahead of time).
       */
      uint16_t maxLogEntries();
      
      /*
       * Returns current number of log entries.
       */
      uint16_t currentLogEntries();

      /*
       * Log a message.
       * Note: this function is purely virtual. It has not been implemented in order to leave the definition of
       *       the message data structure to the consumers of this library. In its implementation, use addLogEntry() 
	   *       to create of a new log entry.
       */
      virtual Timestamp logMessage(MessageID id, int16_t param1, int16_t param2) = 0;
      
      /*
       * Initialises the LogEntry reader to return at most maxResults of the most recent entries.
       * The entries are returned in decending order by timestamp (most recent first).
       * 
       * @param maxResults indicates how many log entries to return as a maximum; the special value 0 means to return all log entries
       * Note: the reader is only valid as long the log is not being modified.
       */
      void readMostRecentLogEntries(uint16_t maxResults);

      /*
       * Initialises the LogEntry reader to return all the log entries that have not yet been notified to the client(s). 
       * The entries are returned in ascending order by timestamp (oldest unnotified entry first).      
       * Note: the reader is only valid as long the log is not being modified.
       */
      void readUnnotifiedLogEntries();

      /*
       * Retuns the "next" entry of the log ("next" can be the next or the previous, depending on the reader kind). 
       * Precondition: readMostRecentLogEntries() or readUnnotifiedLogEntries() was called and the log has not been modified since.
       * 
       * @return true means the parameter 'entry' contains the next log entry, false means 'entry' has no defined semantics (i.e. after the last entry has been returned or if the log has been modified)
       */
      boolean nextLogEntry(LogEntry &entry);

      /*
       * Log a message, halt program execution and blink the universal S-O-S code on the Arduino board's LED.
       * Note: The actual LED pin can be configured / changed via symbol definition (SOS_LED_PIN).
       */
      void log_S_O_S(MessageID id, int16_t param1, int16_t param2);

    protected:
      /*
       * The physical storage of the log.
       */
      AbstractStore *store;
      
      /*
       * The number of slots reserved for log entries in log space.
       * Note: this is the number of slots with differs from 'maximum number' which is the actually available number of slots (one slot is always kept free)
       */
	  uint16_t logEntrySlots;
	  
	  /*
	   * Returns the "magic number" on the store used to identify whether the config area in the storage has been initialised.
	   */
	  uint8_t magicNumber();
	  
  #ifdef UNIT_TEST  // make available for unit tests
    public:
  #endif
      /*
       * Index of the next empty log entry (yet to be written); the entry pointed to has been cleared already
       */
      uint16_t logHeadIndex = 0;
      /*
       * Index of the oldest log entry (there is always one!)
       */
      uint16_t logTailIndex = 0;
      /*
       * Non-concurrent reader (=cursor) to iterate over log entries.
       */
      LogReader reader;
      
  #ifdef UNIT_TEST
    protected:
  #endif
      /*
       * Index of last log entry that was notified to user.
       */
      uint16_t lastNotifiedLogEntryIndex = 0;

      /*
       * Calculates the byte-offset within the logging EEPROM space for the given entry index.
       */
      uint16_t entryOffset(uint16_t index);

      /**
       * Clears the log entry at the current index but does not update logHead or logTail.
       */
      void clearLogEntry(uint16_t index);
      
      /**
       * Creates and adds a log entry at the current logHead position, clears the next entry and updates logHead and logTail.
       */ 
      LogEntry addLogEntry(LogDataTypeID type, LogData *data);
  };
  
      
  /*
   * Write a debug string to Serial, halt program execution and blink the universal S-O-S code on the Arduino board's LED.
   * Note: The actual LED pin can be configured / changed via symbol definition (SOS_LED_PIN).
   * @param line usually pass the source-code line number pseudo variable "__LINE__"
   */
  void write_S_O_S(const __FlashStringHelper *msg, uint16_t line);

#endif
