// T9602_Demo: print one humidity and temperature reading per second.
#include <T9602.h>

T9602 sensor;

void setup() {
    Serial.begin(9600);
    if (!sensor.begin()) {              // false: no sensor answered at 0x28
        Serial.println("T9602 not found");
        while (1);
    }
    Serial.println(sensor.getHeader());  // "Humidity [%],Temp Atmos [C],"
}

void loop() {
    Serial.println(sensor.getString(true));  // take a reading and print it; -9999 on timeout
    delay(1000);
}
