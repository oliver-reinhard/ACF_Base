# ControllerFramework
This library provides an extensible framework to build comprehensive, solid, sensor-based applications for Arduino boards including
* a generic _store_ concept for persistent data (EEPROM, FRAM, in-memory for testing)
* persistent configuration parameters
* persistent logging
* state automata (simple or composite)

Sensor management for OneWire sensors is dealt with by the [ACF_OneWire](https://github.com/oliver-reinhard/ACF_OneWire) library.
A real-world example of this ACF_Base library is the controller for an electrical water boiler (see the [BC_Core](https://github.com/oliver-reinhard/BC_Core) and the [boiler_controller](https://github.com/oliver-reinhard/boiler_controller) main application.

## Installation
Download this library as a .ZIP package, then unzip into the `sketchbook/library`folder of your Arduino workbench, then restart. Alternatively, clone this library to the same location, then restart.

## Dependencies
The framework requires the following libraries to be installed to the libraries folder of the sketchbook of your Arduino workbench:
* ArduinoUnit version >= 2.1 (TODO How to install)
* Adafruit_FRAM_I2C version >= 1.0 (TODO How to install)

**Note**: The fact that these dependency libraries need to be installed into your Arduino workbench does not imply that they  will always included in the sketch uploaded to your Arduino board. They will only be linked if you use the modules that include them.

## Using the Controller Framework
This library contains example programs in the `example` folder for each of the components. Once the library has been installed to your Arduino workbench, you find these example programs in the File > Examples menu. Please also study the test code in the `test` folder for some more ideas about using this framework.

## Components
This section describes the modules contained in this library and their concepts and design ideas.

### ACF_Store
`ACF_Store.h` defines the `AbstractStore` class as a generic but very thin access layer to a physical storage media. `AbstractStore`, as the name suggests, is abstract an cannot be instantiated. Each store has an offset (which can be 0) within the underlying physical media, thus you can create multiple stores over the same media. This is useful if your e.g. EEPROM is used to store configuration values as well as log entries. The store thus uncouples your access from knowing the physical memory addresses.
The `RAMStore` class (an extension of the `AbstractStore`) does not use a persistent media but simply allocates its bytes in the RAM. This is handy for testing, i.e. for writing test cases.

#### ACF_EEPROM
`EEPROMStore` uses the Arduino's EEPROM. Provided your Arduino board actually has an EEPROM (the 32-bit SAMD-based boards don't). Be aware that EEPROM cells have a limited life span in terms of writes, so don't do any high-frequency updates on your EEPROM cells. Anyway, the `EEPROMStore` uses `update` rather than `write` operations wherever possible, thus only writing if the affected bytes actually do change their values.

#### ACF_FRAM
`FRAMStore` uses FRAM (ferro-magnetic RAM) as a persistent media and is the media of choice if either the EEPROM proves to small or if EEPROM is not present at all, like on the 32-bit SAMD-based boards. FRAM is fast an can — from a practical standpoint — be written arbitrarily many times. This implementation is for the Adafruit FRAM board that is accessed via  `Adafruit_FRAM_I2C` library (see Dependencies).

### ACF_Configuration
`ACF_Configuration.h` defines the `AbstractConfigParams` class as a base representation for persistent, but user-changeable or machine-changeable configuration parameters like physical sensor IDs, intervals for logging, etc. 
`AbstractConfigParams` features version numbers for future evolution, i.e. adding more parameters. It also features a "magic number" to enable detecting that the underlying physical store has not been initialised properly or when the offset has shifted (i.e. when the configuration was moved on the phyisical store). In the latter case, the new configuration area will be initilised with default values.
Please read the inline documentation of `ACF_Configuration.h` for the details.

### ACF_LogTime
`ACF_LogTime.h` introduces a 4-byte time format that is able to span more days than by just counting milliseconds (spanning roughly 50 days), yet a better resoulution than counting seconds. 28 bits are used to count seconds (spanning roughly 8.5 years), 4 bits are used to number events within a second (thus 16). Each timestamp provided by this module is guaranteed to be unique. Thus if used for logging, then the timestamps can be used as log-entry identifiers and a maximum of 16 entries per second is possible. Hence this type is not for high-frequency logging but for logging events like state changes and occasional value changes of e.g. temperature readings, etc. If more than 16 timestamps are requested in a given second, then the factory method waits until the second has completed and returns the first timestamp for the next second.

### ACF_Logging
`ACF_Logging.h` implements a circular log using a fixed amount of physical space. When all the space is taken at the end of the log, then space is made at its beginning by clearing and overwriting the oldest entries. The size of the log records is configurable and `ACF_LogTime` is used for unique log-entry identifiers and time stamping.
The `AbstractLog` class is fit for using with EEPROM whose cells only support a limited number of writes (typically around the 100,000 mark): at initialisation time (i.e. at startup) the log detects the start and end positions by physically traversing the log entries, then maintains the two positions in RAM only, thus avoiding to "wear our" position pointers on the EEPROM itself.
`AbstractLog` provides the logging of messages out of the box, identified via `T_Message_ID` identifiers. Other types of log entries (such as state changes) can be added by clients later.
`AbstractLog` maintains a reader object that can be used to notify clients of new log entries.
#### ACF_Messages
Part of the ACF_Logging functionality, all concrete messages logged by the framework itself are defined in `ACF_Messages.h`.

### ACF_State
`ACF_State.h` introduces state and event identifiers, simple and composite state classes, and the `AbstractStateAutomaton` class.
