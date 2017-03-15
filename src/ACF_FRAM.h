#ifndef ACF_FRAM_H_INCLUDED
  #define ACF_FRAM_H_INCLUDED

  #include "Adafruit_FRAM_I2C.h"
  #include <ACF_Store.h>
  
  /*
   * A contiguous part of an Arduino's external Adafruit FRAM I2C storage space.<p>
   *
   * Note: This store must be initialised using init() after creation.
   */
  class FRAMStore : public AbstractStore {
  public:
	  
	  /*
	   * @param offset number of bytesthe first byte of this store is offset from the first byte of the underlying FRAM storage.
	   * @param number of bytes allocated to this store from the underlying FRAM storage space
	   */
	  FRAMStore(const uint32_t offset, const uint32_t size) : AbstractStore(offset, size)  { fram = new Adafruit_FRAM_I2C();}
	  
	  /*
	   * Convenience constructor; allocates storage at offset 0x0.
	   */
	  FRAMStore(const uint32_t size) : FRAMStore((uint32_t) 0, size)  { }
	  
	  /*
	   * Convenience constructor; allocates storage on the same FRAM chip and immediately adjacent to another FRAMStore
	   * beginning at the next higher cell address.
	   */
	  FRAMStore(FRAMStore *predecessor, const uint32_t size) : AbstractStore(predecessor->offset() + predecessor->size(), size)  { fram = predecessor->fram; }
	  
	  /*
	   * Initialise connection to FRAM board.<p>
	   *
	   * Note: Only the first FRAMStore on a given board needs to be initialised.
	   *
	   * @param addr use to set non-default board address (see Adafruit FRAM I2C documentation)
	   * @result return true if connection to FRAM board was successful, else return false.
	   */
	  bool init(uint8_t addr = MB85RC_DEFAULT_ADDRESS);
	  
	  /*
	   * FRAM supports trillions of writes.
	   */
	  bool expiringMedia() { return false; };
      
      /*
       * Set all the bytes allocated to this store to 0x0 on the underlying FRAM storage and reset in-memory storage-managment structures of this store.
       */
      void clear();

      /*
       * Read one byte from the this store.
       * @param idx relative byte offset from the first byte of this store (i.e. not from the first byte of the underlying FRAM storage)
       * @return byte value read from the underlying storage media
       */
      uint8_t read8(uint32_t idx);
    
      /*
       * Write one byte to this store.
       *
       * @param idx relative byte offset from the first byte of this store (i.e. not from the first byte of the underlying FRAM storage )
       * @param val byte value writen to the underlying storage media
       */
      void write8(uint32_t idx, uint8_t val);
    
      /*
       * Semantics equivalent to write().
       */
      bool update8(uint32_t idx, uint8_t val);

    protected:
	  /*
	   * Underlying physical memory.
	   */
	  Adafruit_FRAM_I2C *fram;
  };

#endif
