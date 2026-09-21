#ifndef T9602_h
#define T9602_h

#include "Arduino.h"
#include "Wire.h"

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
	   */
		void begin(uint8_t ADR_ = 0x28); //use default address

    /**
     * @brief Measure relative humidity [%] and temperature [degrees C].
     * @details Sends a measurement request, then fetches data until the
     * sensor's status bits report a valid (not yet fetched) measurement,
     * or until `T9602_TIMEOUT_MS` passes.
     * @return `true` if a valid measurement was read. `false` on timeout
     * (stale or no data, sensor in command mode, or no sensor): the stored
     * values are then -9999.
     */
    bool updateMeasurements();

    /**
     * @brief Status bits from the last data fetch.
     * @return 0: valid data; 1: stale data (already fetched since the last
     * measurement cycle); 2: sensor in command mode (start-up);
     * 3: no data (no sensor on the bus).
     */
    uint8_t getStatus();

    /**
	   * @brief Return the stored relative humidity [%]
	   */
		float getHumidity();

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

	private:
		uint8_t ADR = 0x28; //Default global sensor address
		uint8_t status = 3; //Status bits from the last data fetch
		float RH = -9999;
		float Temp = -9999;
};

#endif
