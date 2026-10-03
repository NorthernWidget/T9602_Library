#ifndef T9602_h
#define T9602_h

#include "Arduino.h"
#include "Wire.h"
#include <NW_Core.h>  //NW_Core: NW_PlainSensor, NW_Readings (statistics), NW_ReadingsConfig, NW_ERROR

//Build identity: this library's version (held equal to library.properties by
//NW-Tests/version_check.py) and its build commit, which the NW-Build wrapper
//sets from git and an Arduino IDE build leaves blank. Both reach a logger's
//status file.
#define T9602_LIBRARY_VERSION "0.0.0"
#ifndef T9602_LIBRARY_COMMIT
#define T9602_LIBRARY_COMMIT ""
#endif

/// Readings per updateMeasurements() are kept in static arrays of this
/// capacity (no heap); setReadings(n) clamps to it. Override before the
/// include to trade RAM for a longer batch.
#ifndef T9602_CAPACITY
#define T9602_CAPACITY 16
#endif

/// Longest wait for valid data in updateMeasurements() [ms]. The ChipCap 2
/// core takes up to 165 ms per measurement cycle in Update mode and up to
/// 55 ms to start up (Amphenol AAS-916-127, Table 5). Override before the
/// include if needed.
#ifndef T9602_TIMEOUT_MS
#define T9602_TIMEOUT_MS 250
#endif

/**
 * @class T9602
 * @brief Library for the IP67-rated T9602 I2C temperature and relative
 *        humidity sensor.
 */
class T9602 : public NW_PlainSensor {
public:
  /** @brief Default I2C address. The part fixes it: there is no strapping pin and no register to change it. */
  static constexpr uint8_t DEFAULT_ADDRESS = 0x28;

  /**
	   * @brief Instantiate the T9602 sensor class
	   */
  T9602();

  /**
	   * @brief Begin communications with the T9602 sensor.
	   * @param[in] ADR_: I2C address. Defaults to 0x28. This library
     *                  cannot change this, but this option exists
     *                  in case you make this change elsewhere.
     * @return `true` if the sensor acknowledges its address on the bus.
	   */
  bool begin(uint8_t ADR_ = DEFAULT_ADDRESS);  //use default address

  /**
     * @brief Measure relative humidity [%] and temperature [degrees C].
     * @details Takes setReadings(n) readings (default one). Each sends a
     * measurement request, then fetches data until the sensor's status bits
     * report a valid (not yet fetched) measurement, or until
     * `T9602_TIMEOUT_MS` passes; readings are therefore independent
     * measurement cycles. The getters return the mean of the readings taken.
     * @return `true` if every reading was valid. `false` if any timed out
     * (stale or no data, sensor in command mode, or no sensor); the stored
     * values are NW_ERROR (-9999) when none was valid.
     */
  bool updateMeasurements();
  /**
     * @brief Set how many readings updateMeasurements() takes (statistics are
     * computed over them). Clamped to T9602_CAPACITY.
     * @return The number actually set.
     */
  uint16_t setReadings(uint16_t n);
  /** @brief Enable or disable humidity and temperature std and sterr columns in printDataHeader()/printDataRow(). */
  void setStats(bool enable);
  /** @brief Number of valid readings stored by the last updateMeasurements(). */
  uint16_t getReadingCount();

  /**
     * @brief Status bits from the last data fetch.
     * @return 0: valid data; 1: stale data (already fetched since the last
     * measurement cycle); 2: sensor in command mode (start-up);
     * 3: no data (no sensor on the bus).
     */
  uint8_t getStatus();

  /**
	   * @brief Return the stored relative humidity [%]: the mean of the last
	   * updateMeasurements(), NW_ERROR (-9999) when none was valid.
	   */
  float getHumidity();

  //--- Statistics getters ---
  //Computed two-pass in 32-bit float over the readings stored by the last
  //updateMeasurements() (NW_Readings). NW_ERROR when there are none.
  /** @brief Humidity mean [%] over the stored readings. */
  float getHumidityMean();
  /** @brief Humidity standard deviation [%]. */
  float getHumidityStd();
  /** @brief Humidity standard error [%]. */
  float getHumiditySterr();
  /** @brief Humidity median [%] (mean of the middle pair for even N). */
  float getHumidityMedian();
  /** @brief Temperature mean [C] over the stored readings. */
  float getTemperatureMean();
  /** @brief Temperature standard deviation [C]. */
  float getTemperatureStd();
  /** @brief Temperature standard error [C]. */
  float getTemperatureSterr();
  /** @brief Temperature median [C]. */
  float getTemperatureMedian();

  /**
	   * @brief Return the stored temperature [degrees C]
	   */
  float getTemperature();

  /**
		 * @brief Print the summary columns a logger writes: the means, with the
		 * statistics columns when they are enabled.
		 * @details "Humidity [%],Temp Atmos [C]," with std and sterr columns
		 * after each value when setStats(true) is on. Pass a `File` to write the
		 * card, `Serial` to write the monitor. Distinct from printHeader(), which
		 * is the burst interface and carries no statistics.
		 * @param out Where to print.
		 * @return Bytes printed.
		 */
  size_t printDataHeader(Print& out) override;

  /**
		 * @brief Print one summary row, in printDataHeader()'s column order.
		 * @details Takes no reading: it prints what the last updateMeasurements()
		 * left, which is what lets a caller write the same row to two sinks
		 * without acquiring twice.
		 * @param out Where to print.
		 * @return Bytes printed.
		 */
  size_t printDataRow(Print& out) override;

  //--- NW_Sensor, through NW_PlainSensor: what a logger asks of a sensor ---
  //The T9602 has no Page 0 and no Report register, so it names itself here
  //rather than on the bus and NW_Logger::discover() cannot find it. A sketch
  //names it and watch() logs its columns like any other sensor's.

  /** @brief The part's name, for the status file and for a sketch's own use. */
  const char* name() const override {
    return "T9602";
  }

  /** @brief The address it answers at unless the logger says otherwise. */
  uint8_t defaultAddress() const override {
    return DEFAULT_ADDRESS;
  }

  /** @brief Come back on the bus after the logger cut the sensor rail to sleep. */
  bool wake(uint8_t address) override {
    return begin(address);
  }

  /** @brief Take this row's readings, for printDataRow() to print. */
  bool acquire() override {
    return updateMeasurements();
  }

  /**
	   * @brief Dummy function to enable sleep mode.
     * @details Currently not used. Instead, we simply power the sensor down.
	   */
  bool sleep();

  //--- Reading interface (NW standard) ---
  /**
     * @brief Print the header matching printReading(): "Humidity [%],Temp Atmos [C],".
     * No statistics columns: one reading has none.
     * @param out Any Print destination (SdFat File, Serial, ...).
     * @return Bytes written.
     */
  size_t printHeader(Print& out);
  /**
     * @brief Print the stored reading, each value followed by a comma. Does
     * not acquire: call updateMeasurements() first, or use logReading().
     * @return Bytes written.
     */
  size_t printReading(Print& out);
  /**
     * @brief Take ONE reading and print it: the one-reading primitive for
     * collecting many readings to a file.
     * @return Bytes written.
     */
  size_t logReading(Print& out);
  /**
     * @brief Begin a run of readings. The T9602 is not a Schema 1 device, so
     * n is only the number of logReading() calls to follow; nothing is
     * declared to the sensor.
     */
  void beginReadings(uint16_t n = 0);
  /** @brief End a run of readings. */
  void endReadings();

private:
  uint8_t ADR = 0x28;   //Default global sensor address
  uint8_t status = 3;   //Status bits from the last data fetch
  float RH = NW_ERROR;  //Mean of the last updateMeasurements()
  float Temp = NW_ERROR;
  bool readOnce();                           //One measurement cycle, appended to the readings
  NW_Readings<float, T9602_CAPACITY> _rh;    //[%]
  NW_Readings<float, T9602_CAPACITY> _temp;  //[C]
  NW_ReadingsConfig _cfg;                    //Readings per updateMeasurements() and the stats columns
};

#endif
