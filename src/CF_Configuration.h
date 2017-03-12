#ifndef CF_CONFIGURATION_H_INCLUDED
  #define Cf_CONFIGURATION_H_INCLUDED

  #include <CF_Store.h>

  /*
   * The storage structure for configuration parameters is as follows:
   * 
   * - Magic number (1 byte) -- enables detectiton whether the config area has been written before
   * - Version (1 byte) -- enables detection of structural changes of the config area
   * - Parameter values (n bytes)  
   * 
   * This class implements version number storage and provides a base for extension for
   * your own parameters. When this configuration structure is first saved, the required
   * space on the store is initialized with 0x0. Parameter values are then read from the
   * 0x0-initialized store (thus resulting in zero-values) from which they will be
   * initialized with your own default values (you need to implement initParams() for that). 
   * Then these new values are stored.
   *
   * It is thus recommendable to dimension the original config store object *large enough* for future extensions
   * with additional parameter values. The storage area that is not being used initially will also be initialised 
   * with 0x0; and if more parameters are added later they will be initialised with default values.
   *
   * The RAM layout of subclasses of AbstractConfigParams is as follows:
   *
   * - Superclass pointer (2 bytes)
   * - Pointer to AbstractStore (2 bytes)
   * - Layout version (1 byte)
   * - (actual configuration parameter values (n bytes))
   */
  class AbstractConfigParams {

    public:
      /*
       * @param eepromOffset number of bytes this object's storage is offset from the first byte of the EEPROM store.
       * @param layoutVersion static version identifier of the config data structure. Should be increased whenever the number or parameter,  their types or sizes change.
       */
      AbstractConfigParams(AbstractStore *store, const uint8_t version);
      
      /*
       * Returns the version identifier.
       */
      uint8_t version();
      
      /*
       * Clears magic number, version and all config-parameter values stored on the EEPROM by writing 0x0 to each memory cell.
       */
      void clear();

      /*
       * Loads  parameter values from the EEPROM and applies default values to uninitialised parameters.
       */
      void load();
      
      /*
       * Saves (changed) parameter values to the EEPROM.
       */
      void save();

      /*
       * Print config parameter values to Serial.<p>
	   *
       * Note: Consumers should override and invoke "super" first but implementation is optional.
       */
      void print();
      
      /*
       * Dynamic version of sizeof() (object size in memory); works for subclasses, too.
       */
      virtual uint16_t memSize() = 0;

   protected:
	  /*
	   * Used storage area.
	   */
	  AbstractStore *store;

      /*
       * Version identifier of the config data structure.
       */
      uint8_t layoutVersion;

      /*
       * Returns the "magic number" on the store used to identify whether the config area in the storage has been initialised.
       */
	  uint8_t magicNumber();
	  
	  /*
	   * Ensures every parameter has been set to a value different from 0 and, if not, sets it to its default value.
	   * @param updated tells, whether any values were affected.
	   */
	  virtual void initParams(boolean &updated) = 0;
	  
      /*
       * Reads the configuration values (and only these) from the EEPROM. No initialisation of values is performed.
       */
      void readParams();
  };


#endif // BC_CONFIGURATION_H_INCLUDED
