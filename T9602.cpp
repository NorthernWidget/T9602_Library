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

float T9602::GetHumidity()  //Return humidity in % (realtive)
{
	float RH = 0; //Calulated RH value
	uint8_t Data[2] = {0}; //Array for raw data from device

	Wire.beginTransmission(ADR);
	Wire.write(0x00);  //Read only upper 2 data bytes
	Wire.endTransmission();

	Wire.requestFrom(ADR, 2);
	for(int i = 0; i < 2; i++) { //Read in raw data
		Data[i] = Wire.read();
	}
	RH = (float)((((Data[0] & 0x3F ) << 8) + Data[1]) / 16384.0) * 100.0; //Convert RH
	return RH;
}

float T9602::GetTemperature()  //Return temp in C
{
	float Temp = 0; //Calulated temp value
	uint8_t Data[2] = {0}; //Array for raw data from device

	Wire.beginTransmission(ADR);
	Wire.write(0x02);  //Read only from lower 2 data bytes
	Wire.endTransmission();

	Wire.requestFrom(ADR, 2);
	for(int i = 0; i < 2; i++) { //Read in raw data
		Data[i] = Wire.read();
	}

	Temp = (float)((unsigned((Data[2] * 64)) + unsigned((Data[3] >> 2 ))) / 16384.0) * 165.0 - 40.0;  //Convert Temp
	return Temp;
}

String T9602::GetHeader()
{
	return "Humidity [%],Temp Atmos [C],";
}

String T9602::GetString()
{
	float Temp = 0; //Calulated temp value
	float RH = 0; //Calulated RH value
	// uint8_t Data[4] = {0}; //Array for raw data from device
	uint8_t Data[4] = {0}; //DEBUG!

	Wire.beginTransmission(ADR);
	Wire.write(0x00);
	Wire.endTransmission();

	Wire.requestFrom(ADR, 4);
	for(int i = 0; i < 4; i++) { //Read in raw data
		Data[i] = Wire.read();
	}
	RH = (float)((((Data[0] & 0x3F ) << 8) + Data[1]) / 16384.0) * 100.0; //Convert RH
	// RH = (float)(((Data[0] & 0x3F ) << 8) + Data[1]) / 16384.0 * 100.0; //DEBUG!
	Temp = (float)((unsigned((Data[2] * 64)) + unsigned((Data[3] >> 2 ))) / 16384.0) * 165.0 - 40.0;  //Convert Temp
	// Temp = (float)((unsigned)(Data[2]  * 64) + (unsigned)(Data[3] >> 2 )) / 16384.0 * 165.0 - 40.0;  //DEBUG!

	return String(RH) + "," + String(Temp) + ",";
}

bool T9602::Sleep()
{
	//Add sleep command
}
