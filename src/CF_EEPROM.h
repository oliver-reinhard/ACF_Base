#ifndef CF_EEPROM_H_INCLUDED
  #define CF_EEPROM_H_INCLUDED

  #include <EEPROM.h>
  #include <CF_Store.h>
  
  /*
   * A contiguous part of the Arduino's EEPROM storage space.<p>
   *
   * Note: Not all Arduino boards have EEPROM storage (e.g. it's missing from the Adafruit Feather M0).
   */
  class EEPROMStore : public AbstractStore {
    public:
      
      /*
       * @param offset number of bytesthe first byte of this store is offset from the first byte of the underlying EEPROM storage
       * @param number of bytes allocated to this store from the underlying EEPROM storage space
       */
	  EEPROMStore(const uint32_t offset, const uint32_t size) : AbstractStore(offset, size)  { }
	  
	  /*
	   * Convenience constructor that grabs the remaining space on the EEPROM.
	   */
	  EEPROMStore(const uint32_t offset) : AbstractStore(offset, EEPROM.length() - offset)  { }
	  
	  /*
	   * Convenience constructor; allocates storage on the same FRAM chip and immediately adjacent to another EEPROMStore
	   * beginning at the next higher cell address.
	   */
	  EEPROMStore(EEPROMStore *predecessor, const uint32_t size) : EEPROMStore(predecessor->offset() + predecessor->size(), size)  { }
	  
	  /*
	   * EEPROM supports only several 10.000 writes.
	   */
	  bool expiringMedia() { return true; };
	  
      /*
       * Set all the bytes allocated to this store to 0x0 on the underlying EEPROM storage and reset in-memory storage-managment structures of this store.
       */
      void clear();

      /*
       * Read one byte from the this store.
       * @param idx relative byte offset from the first byte of this store (i.e. not from the first byte of the underlying EEPROM storage)
       * @return byte value read from the underlying storage media
       */
      uint8_t read8(uint32_t idx);
    
      /*
       * Write one byte to this store.<p>
       * Important: EEPROM cells have a limited lifespan i.e, a limited number of writes. Use update() if possible.
	   *
       * @param idx relative byte offset from the first byte of this store (i.e. not from the first byte of the underlying EEPROM storage )
       * @param val byte value writen to the underlying storage media
       */
      void write8(uint32_t idx, uint8_t val);
    
      /*
       * Write one byte to this store if (and only if) the value currently stored value at the given offset is different. Prefer this function to write().
	   *
       * @param idx relative byte offset from the first byte of this store (i.e. not from the first byte of the underlying EEPROM storage)
	   * @param val byte value writen to the underlying storage media
	   * @result return true if val is different from current value, i.e. cell was changed, false otherwise
       */
      bool update8(uint32_t idx, uint8_t val);
  };

#endif
