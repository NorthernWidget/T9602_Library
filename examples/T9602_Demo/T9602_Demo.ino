// T9602_Demo: one row per second from a T9602 temperature and relative
// humidity sensor over I2C. Header once, then a reading and a row each loop.
#include <T9602.h>

T9602 sensor;

void setup() {
    Serial.begin(9600);
    if (!sensor.begin()) {
        Serial.println("T9602 not found. Check wiring.");
        while (1);
    }
    sensor.printDataHeader(Serial);  // "Humidity [%],Temp Atmos [C],"
    Serial.println();
}

void loop() {
    sensor.updateMeasurements();     // take the reading; printDataRow() prints what it left
    sensor.printDataRow(Serial);     // -9999 on timeout
    Serial.println();
    delay(1000);
}
