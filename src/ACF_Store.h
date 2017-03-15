#ifndef ACF_STORE_H_INCLUDED
  #define ACF_STORE_H_INCLUDED
  
  #include <Arduino.h>
  
  /*
   * A contigous part of an Arduino storage media such as EEPROM, FRAM, SD, etc..
   */
  class AbstractStore {   
    public:
      
      /*
       * @param offset number of bytesthe first byte of this store is offset from the first byte of the underlying storage media.
       * @param size number of bytes allocated to this store from the underlying storage-media space
       */
      AbstractStore(const uint32_t offset, const uint32_t size)  { this->offsetBytes = offset; this->sizeBytes = size; }

      /*
       * Return the byte offset of this store with respect to the underlying storage media.
       */
      uint32_t offset() { return offsetBytes; }
      
      /*
       * Return the number of bytes allocated to this store on the underlying storage media.
       */
      uint32_t size() { return sizeBytes; }
	  
	  /*
	   * Return true if the underlying storage media supports only a limited number of write operations per cell (like e.g. EEPROM), else return false.
	   */
	  virtual bool expiringMedia() = 0;
      
      /*
       * Set all the bytes allocated to this store to 0x0 on the underlying storage media and reset in-memory storage-managment structures of this store.
       */
      virtual void clear() = 0;

      /*
       * Read one byte from the this store.
       * @param idx relative byte offset from the first byte of this store (i.e. not from the first byte of the underlying storage media)
       * @return byte value read from the underlying storage media
       */
      virtual uint8_t read8(uint32_t idx) = 0;
    
      /*
       * Read a multi-byte object from the this store.
       * @param idx relative byte offset from the first byte of this store (i.e. not from the first byte of the underlying storage media)
       * @param obj object to read from the underlying storage media; read sizeof(T) bytes
       * @return obj as passed as second parameter
       */
      template<typename T> T &read(uint32_t idx, T &obj){
        uint8_t *ptr = (uint8_t*) &obj;
        const uint32_t len = sizeof(T);
        for (uint32_t i=0; i<len; i++) *ptr++ = read8(idx+i);
        return obj;
      }
    
      /*
       * Write one byte to this store (unconditionally).
       * @param idx relative byte offset from the first byte of this store (i.e. not from the first byte of the underlying storage media)
       * @param val byte value writen to the underlying storage media
       */
	  virtual void write8(uint32_t idx, uint8_t val) = 0;
	  
	  /*
	   * Write a multi-byte object to this store (unconditionally).
	   * @param idx relative byte offset from the first byte of this store (i.e. not from the first byte of the underlying storage media)
	   * @param obj object to write to the underlying storage media; write sizeof(T) bytes
	   * @return obj as passed as second parameter
	   */
	  template<typename T> void write(uint32_t idx, const T &obj){
		  const uint8_t *ptr = (const uint8_t*) &obj;
		  const uint32_t len = sizeof(T);
		  for (uint32_t i=0; i<len; i++)  write8(idx+i, *ptr++);
	  }
	  
      /*
       * Write one byte to this store if (and only if) the value currently stored at the given offset is different. This is important for
       * storage media whose storage cells have a limited lifespan, i.e. a limited number of writes like e.g. EEPROM. For non-expiring media update() is equivalent to write().
       * @param idx relative byte offset from the first byte of this store (i.e. not from the first byte of the underlying storage media)
       * @param val byte value writen to the underlying storage media
	   * @result return true if val is different from current value, i.e. cell was changed, false otherwise
       */
      virtual bool update8(uint32_t idx, uint8_t val) = 0;
     
      /*
       * Write a multi-byte object to this store. Only changed bytes are written. This is important for storage media whose storage cells have a limited lifespan, 
       * i.e. a limited number of writes like e.g. EEPROM. 
       * @param idx relative byte offset from the first byte of this store (i.e. not from the first byte of the underlying storage media)
	   * @param obj object to write to the underlying storage media; write sizeof(T) bytes
	   * @result return true if obj is different from currently stored value, i.e. one or more bytes were changed, false otherwise
	   */
      template<typename T> bool update(uint32_t idx, const T &obj){
        const uint8_t *ptr = (const uint8_t*) &obj;
        const uint32_t len = sizeof(T);
		bool updated = false;
        for (uint32_t i=0; i<len; i++)  updated |= update8(idx+i, *ptr++);
        return updated;
      }

    protected:
      /*
       * Number of bytes the first byte of this store is offset from the first byte of the underlying storage media.
       */
      uint32_t offsetBytes;
      /*
       * Number of bytes allocated to this store from the underlying storage-media space.
       */
      uint32_t sizeBytes;
  };

  /*
   * Store implemented as a non-persistend memory object. Use for testing purposes.
   */
  class RAMStore : public AbstractStore {
    public:
      RAMStore(const uint32_t size) : AbstractStore(0, size) { memory = (uint8_t*) malloc(size); }
      ~RAMStore() { free(memory); }
	  
	  bool expiringMedia() { return false; };
	  void clear();
      uint8_t read8(uint32_t idx);
      void write8(uint32_t idx, uint8_t val);
      bool update8(uint32_t idx, uint8_t val);

   protected:
     uint8_t *memory;
  };

#endif
