#ifndef T9602_h
#define T9602_h

#include "Arduino.h"
#include "Wire.h"
#include <NW_Core.h>   //NW_Core: NW_Readings (statistics), NW_ReadingsConfig, NW_ERROR

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
class T9602
{
	public:
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
		bool begin(uint8_t ADR_ = 0x28); //use default address

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
    /** @brief Enable or disable humidity and temperature std and sterr columns in getString()/getHeader(). */
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
	   * @brief The most important function for the user! Returns all data as a
	   * comma-separated string: "RH,T,".
	   * @details This string is: "RELATIVE_HUMIDITY,TEMPERATURE,".
     * It is written with the code: return String(RH) + "," + String(Temp) + ","
     * @param[in] takeNewReadings: if `true` run `updateMeasurements` before
     * returning values. Otherwise, just return values.
     */
		String getString(bool takeNewReadings = false);

    /**
	   * @brief Return the header as an Arduino string:
     * "Relative Humidity [%],Temp Atmos [C],"
	   */
		String getHeader();

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
		uint8_t ADR = 0x28; //Default global sensor address
		uint8_t status = 3; //Status bits from the last data fetch
		float RH = NW_ERROR; //Mean of the last updateMeasurements()
		float Temp = NW_ERROR;
		bool readOnce(); //One measurement cycle, appended to the readings
		NW_Readings<float, T9602_CAPACITY> _rh; //[%]
		NW_Readings<float, T9602_CAPACITY> _temp; //[C]
		NW_ReadingsConfig _cfg; //Readings per updateMeasurements() and the stats columns
};

#endif
