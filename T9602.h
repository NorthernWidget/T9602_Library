#ifndef T9602_h
#define T9602_h

#include "Arduino.h"
#include "Wire.h"

class T9602
{
	public:
		T9602();
		uint8_t begin(uint8_t ADR_ = 0x28); //use default address
		float GetHumidity();
		float GetTemperature();
		String GetString();
		String GetHeader();
		bool Sleep();
	private:
		uint8_t ADR = 0x28; //Default global sensor address 
};

#endif