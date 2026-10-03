[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.6338378.svg)](https://doi.org/10.5281/zenodo.6338378)

# T9602_Library

***Arduino library interface for the [Telaire T9602 humidity and temperature sensor](https://www.amphenol-sensors.com/en/telaire/humidity/527-humidity-sensors/3224-t9602)***

![T9602 from Telaire](https://f.hubspotusercontent40.net/hubfs/9035299/main-T9602-Mod-4.png)

***T9602 sensor.*** *Image from Amphenol/Telaire (see link above).*

## Library

**Full API reference:** https://docs.northernwidget.com/T9602_Library/ (Doxygen, built from `master` by GitHub Actions).

## Key sensor features

  * IP67 rated
  * -20 to 70 °C operating range
  * Accuracy
    * ±2% RH, 0-95% RH
    * ±0.5°C
  * 14 bit resolution

[Data sheet](https://www.amphenol-sensors.com/en/component/edocman/304-telaire-t9602-humidity-temperature-sensor-datasheet/download?Itemid=8487%20%27) for the full T9602 series.

[Application guide for the ChipCap® 2 Humidity and Temperature Sensor](https://www.amphenol-sensors.com/en/component/edocman/397-telaire-chipcap2-humidity-and-temperature-sensor-application-guide/download?Itemid=8487%20%27) (core unit within the T9602). Importantly, this includes a guide to the I2C bus. This library operates only using the default I2C address (0x28), so you will need to consult this and this library (and make a pull request, please!) if you want to allow for different addresses to be set/used.

## Sensor options

Our two most commonly used sensors are:
  * [T9602-3-D](https://www.digikey.com/product-detail/en/amphenol-advanced-sensors/T9602-3-D/235-1563-ND/5027914)
    * I2C
    * 3.3V
    * 1.8 m cable
  * [T9602-3-D-1](https://www.digikey.com/product-detail/en/amphenol-advanced-sensors/T9602-3-D-1/235-1377-ND/5027895)
    * I2C
    * 3.3V
    * 1.0 m cable

5V versions, as well as those with other communications protocols, [are available as well](https://www.digikey.com/products/en/sensors-transducers/humidity-moisture-sensors/529?k=t9602).

## Wiring

**_WARNING_: LAYOUT DIFFERS FROM OUR STANDARD CONVENTIONS**

| **Color** | **Connection** |
|-----------|----------------|
| Red       | V+             |
| Green     | GND            |
| White     | SDA            |
| Black     | SCL            |

We typically snip off the connector at and wire the sensor directly into the screw terminals of our data logger ([Resnik](https://github.com/NorthernWidget-Skunkworks/Project-Resnik) or [Margay](https://github.com/NorthernWidget-Skunkworks/Project-Margay)) or into our [Heptapod](https://github.com/NorthernWidget-Skunkworks/Project-Heptapod/) I2C expander. You may instead choose to find a mating plug; the wires feel a bit delicate, but in our experience hold up better than expected.

![T9602 Digi-Key image with connector](https://media.digikey.com/photos/Amphenol%20Photos/T9602%20SERIES.jpg)

***T9602 and connector.*** *Image from Digi-Key (see links above).*

## Writing a program to connect to the T9602

You should be able to use any standard Arduino device to connect to the T9602 and read its data.

*This code is untested. If you do test it, please confirm that it works and/or create a PR and/or an issue for it to be fixed.*

### Very simple Arduino code

This code is intended for any generic Arduino system. It is not proven.

```c++
// Include the T9602 library
#include "T9602.h"

// Instantiate class
T9602 mySensor;

void setup(){
    // Begin Serial connection to computer at 38400 baud
    Serial.begin(38400);
    // Print the header just once, straight to the port
    mySensor.printDataHeader(Serial);
    Serial.println();
}

void loop(){
    // Take one reading every (10 + time to take reading) seconds
    // and print it to the screen
    mySensor.updateMeasurements();
    mySensor.printDataRow(Serial);
    Serial.println();
    delay(10000); // Wait 10 seconds before the next reading, inefficiently
}
```

`updateMeasurements()` sends a measurement request and then fetches data until the sensor's status bits report a valid, not-yet-fetched measurement. It returns `true` on success and `false` after `T9602_TIMEOUT_MS` (250 ms by default; define it before the include to change it) without valid data, in which case the stored values are NW_ERROR (-9999). `getStatus()` returns the last status bits: 0 valid, 1 stale (already fetched since the last measurement cycle), 2 command mode (the sensor is still starting up), 3 no data (no sensor answered). `printDataRow(out)` prints what the last `updateMeasurements()` left, and takes no reading of its own, so the same row can go to the card and the monitor without measuring twice. Both it and `printDataHeader(out)` write into any `Print`: no row is composed in RAM.

`setReadings(n)` sets how many readings `updateMeasurements()` takes (each is its own measurement cycle, so they are independent; clamped to `T9602_CAPACITY`, default 16, override before the include); the getters then return the means, and `getHumidityMean()`, `getHumidityStd()`, `getHumiditySterr()`, `getHumidityMedian()`, the same for temperature, and `getReadingCount()` read the stored readings. With `setStats(true)` the std and sterr columns join `printDataHeader()` and `printDataRow()`. For one row per reading to a file, `beginReadings(n)`, `printHeader(out)`, `logReading(out)` n times, `endReadings()`, to any `Print` (an SdFat `File`, `Serial`). Requires the [NW_Core](https://github.com/NorthernWidget/NW_Core) library.

### Northern Widget Margay code

The [Margay data logger](github.com/NorthernWidget-Skunkworks/Project-Margay) is the lightweight and low-power open-source data-logging option from Northern Widget. It saves data to a local SD card and includes on-board status measurements and a low-drift real-time clock. We have written [a library to interface with the Margay](github.com/NorthernWidget-Skunkworks/Margay_Library), which can in turn be used to link the Margay with sensors.

**A Margay cannot yet hold a T9602.** A logger writes its file from the sensors `watch()` gave it, and `watch()` takes an `NW_Sensor`: a device that names itself in Page 0 and reports through the Schema 1 Report register ([LIBRARY-DESIGN.md](https://github.com/NorthernWidget/NW-Device-Specification/blob/master/LIBRARY-DESIGN.md) section 14). The T9602 is an Amphenol part with neither, so this sketch logs the logger's own on-board columns and prints the T9602's to the serial monitor; its columns do not reach the card. Read that as the open end of the work rather than as the intended shape.

```c++
// Include the T9602 library
#include <Margay.h>
#include <T9602.h>

// Instantiate classes
T9602 mySensor;
Margay Logger(MODEL_2v0, BUILD_B); // Margay v2.2; UPDATE CODE TO INDICATE THIS

//Number of seconds between readings
uint32_t updateRate = 60;

void setup(){
    mySensor.begin();
    mySensor.printDataHeader(Serial);
    Serial.println();
    Logger.begin();
}

void loop(){
    Logger.run(updateRate);
}
```

## Testing

`extras/test/run.sh` compiles the library on a desktop against stub `Arduino.h` and `Wire.h` that emulate the ChipCap 2 status bits, and checks that the output for fixed sensor behaviour is byte-identical to `extras/test/baseline.txt`. Run it after any change; `--record` rewrites the baseline when an output change is intended.

## Acknowledgments

Support for this project provided by:

<img src="https://pbs.twimg.com/profile_images/1445421246045360133/zQtKhpkT_400x400.jpg" alt="UMN ESCI" width="240px">

<img src="https://www.nsf.gov/news/mmg/media/images/nsf_logo_f_ba321daf-8607-41d7-94bc-1db6039d7893.jpg" alt="NSF" width="240px">
