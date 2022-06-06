#include "T9602.h"

//FIX! Add status bit testing!
T9602::T9602()
{
}

uint8_t T9602::begin(uint8_t ADR_)
{
	ADR = ADR_;
	Wire.begin();
}

void T9602::updateMeasurements(){

  // Relative humidity
	uint8_t data[4] = {0}; //Array for raw data from device

	Wire.beginTransmission(ADR);
	Wire.write(0);  //Wake up and commence conversions
	Wire.endTransmission();

	Wire.requestFrom(ADR, 4);
	for(int i = 0; i < 4; i++) { //Read in raw data
		data[i] = Wire.read();
	}
	// Convert RH to percent
	RH = (float)(((data[0] & 0x3F ) << 8) + data[1]) / 16384.0 * 100.0; //Convert RH
	// Temperature
	Temp = (float)((unsigned)(data[2] * 64) + (unsigned)(data[3] >> 2 )) / 16384.0 * 165.0 - 40.0;  //Convert Temp
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
	return String(RH) + "," + String(Temp) + ",";
}

bool T9602::sleep()
{
	//Add sleep command
}

