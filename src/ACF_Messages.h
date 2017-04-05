#ifndef ACF_MESSAGES_H
  #define ACF_MESSAGES_H
  
  #include <ACF_Types.h>

  /*
   * The messages issued by the implementation of this module. Stored as T_Message_ID.
   * Note: the actual message texts and the conversion of message IDs to text have to be implemented by 
   *       the consumer of this libraray 
   */
  enum class ACF_Msg : T_Message_ID {
    SYSTEM_INIT         = 1,  // The board is initialising from reset or power on [no parameters]
    
    LOG_INIT            = 2,  // Log initialised [no parameters]
    LOG_MAGIC_NUMBER    = 3,  // Log was cleared because magic number was not detected ->
    LOG_SIZE_CHG        = 4,  // Log was cleared because number of log entries has changed from [old] to [new])
    
    STATE_ILLEGAL_TRANS = 5,  // State [state]: illegal transition attemt (event [event])
    STATE_UNKNOWN_STATE = 6   // State [state] has not been defined
  };

#endif
