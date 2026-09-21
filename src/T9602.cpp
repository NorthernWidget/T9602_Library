#include "T9602.h"

T9602::T9602()
{
}

void T9602::begin(uint8_t ADR_)
{
	ADR = ADR_;
	Wire.begin();
}

bool T9602::updateMeasurements(){

	uint8_t data[4] = {0}; //Array for raw data from device

	// Measurement Request (address + write; trailing data is "don't care").
	// Starts a cycle on a Sleep-mode part; the T9602 ships in Update mode,
	// where the sensor measures on its own and this is a no-op.
	Wire.beginTransmission(ADR);
	Wire.write(0x00);
	Wire.endTransmission();

	// Data Fetch until the status bits (7:6 of the first byte) read 00 = valid.
	// 01 = stale: already fetched since the last measurement cycle;
	// 10 = command mode (start-up); 11 = no data (no sensor: reads give 0xFF).
	uint32_t start = millis();
	while(true) {
		Wire.requestFrom(ADR, 4);
		for(int i = 0; i < 4; i++) { //Read in raw data
			data[i] = Wire.read();
		}
		status = data[0] >> 6;
		if(status == 0) break;
		if(millis() - start >= T9602_TIMEOUT_MS) {
			RH = -9999;
			Temp = -9999;
			return false;
		}
		delay(2);
	}

	// Convert RH to percent
	RH = (float)((((data[0] & 0x3F ) << 8) + data[1]) / 16384.0) * 100.0; 
	// Convert Temp
	Temp = (float)((unsigned((data[2] * 64)) + unsigned((data[3] >> 2 ))) / 16384.0) * 165.0 - 40.0;  
	return true;
}

uint8_t T9602::getStatus()
{
	return status;
}

float T9602::getHumidity()  //Return humidity in % (realtive)
{
	return RH;
}

float T9602::getTemperature()  //Return temp in C
{
	return Temp;
}

String T9602::getHeader()
{
	return "Humidity [%],Temp Atmos [C],";
}

String T9602::getString(bool takeNewReadings)
{
	if(takeNewReadings) updateMeasurements();
	return String(RH) + "," + String(Temp) + ",";
}

bool T9602::sleep()
{
	//Add sleep command
	return false;
}

