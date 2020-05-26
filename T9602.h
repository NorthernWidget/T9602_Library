#ifndef T9602_h
#define T9602_h

#include "Arduino.h"
#include "Wire.h"

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
		uint8_t begin(uint8_t ADR_ = 0x28); //use default address

    /**
	   * @brief Return the current relative humidity [%]
	   */
		float GetHumidity();

    /**
	   * @brief Return the current temperature [degrees C]
	   */
		float GetTemperature();

    /**
	   * @brief The most important function for the user! Returns all data as a
	   * comma-separated string: "RH,T,".
	   * @details This string is: "RELATIVE_HUMIDITY,TEMPERATURE,".
     * It is written with the code: return String(RH) + "," + String(Temp) + ","
     */
		String GetString();

    /**
	   * @brief Return the header as an Arduino string:
     * "Humidity [%],Temp Atmos [C],"
	   */
		String GetHeader();

    /**
	   * @brief Dummy function to enable sleep mode.
     * @details Currently not used. Instead, we simply power the sensor down.
	   */
		bool Sleep();

	private:
		uint8_t ADR = 0x28; //Default global sensor address
};

#endif
