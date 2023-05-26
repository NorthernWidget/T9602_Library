#include "T9602.h"

//FIX! Add status bit testing!
T9602::T9602()
{
}

void T9602::begin(uint8_t ADR_)
{
	ADR = ADR_;
	Wire.begin();
}

void T9602::updateMeasurements(){

  // Relative humidity
	uint8_t data[2] = {0}; //Array for raw data from device

	Wire.beginTransmission(ADR);
	Wire.write(0x00);  //Read only upper 2 data bytes
	Wire.endTransmission();

	Wire.requestFrom(ADR, 2);
	for(int i = 0; i < 2; i++) { //Read in raw data
		data[i] = Wire.read();
	}

	// Convert RH to percent
	RH = (float)((((data[0] & 0x3F ) << 8) + data[1]) / 16384.0) * 100.0; 


  // Temperature
  
  // Reset the array to zeros
	data[0] = 0.;
	data[1] = 0.;

	Wire.beginTransmission(ADR);
	Wire.write(0x02);  //Read only from lower 2 data bytes
	Wire.endTransmission();

	Wire.requestFrom(ADR, 2);
	for(int i = 0; i < 2; i++) { //Read in raw data
		data[i] = Wire.read();
	}

	Temp = (float)((unsigned((data[2] * 64)) + unsigned((data[3] >> 2 ))) / 16384.0) * 165.0 - 40.0;  //Convert Temp
	
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
	return false;
}

