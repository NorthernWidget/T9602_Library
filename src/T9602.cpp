#include "T9602.h"

T9602::T9602() {
}

bool T9602::begin(uint8_t ADR_) {
  ADR = ADR_;
  Wire.begin();
  Wire.beginTransmission(ADR);
  return Wire.endTransmission() == 0;  //ACK?
}

bool T9602::updateMeasurements() {
  //N independent measurement cycles, then the means; NW_ERROR when none was valid.
  _rh.reset();
  _temp.reset();
  for (uint16_t i = 0; i < _cfg.n; i++) readOnce();
  RH = _rh.mean();  //NW_ERROR when empty
  Temp = _temp.mean();
  return _cfg.n > 0 && _rh.count() == _cfg.n;
}

uint16_t T9602::setReadings(uint16_t n) {
  return _cfg.set(n, T9602_CAPACITY);
}
void T9602::setStats(bool enable) {
  _cfg.stats = enable;
}
uint16_t T9602::getReadingCount() {
  return _rh.count();
}

float T9602::getHumidityMean() {
  return _rh.mean();
}
float T9602::getHumidityStd() {
  return _rh.std();
}
float T9602::getHumiditySterr() {
  return _rh.sterr();
}
float T9602::getHumidityMedian() {
  return _rh.median();
}
float T9602::getTemperatureMean() {
  return _temp.mean();
}
float T9602::getTemperatureStd() {
  return _temp.std();
}
float T9602::getTemperatureSterr() {
  return _temp.sterr();
}
float T9602::getTemperatureMedian() {
  return _temp.median();
}

bool T9602::readOnce() {

  uint8_t data[4] = { 0 };  //Array for raw data from device

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
  while (true) {
    Wire.requestFrom(ADR, 4);
    for (int i = 0; i < 4; i++) {  //Read in raw data
      data[i] = Wire.read();
    }
    status = data[0] >> 6;
    if (status == 0) break;
    if (millis() - start >= T9602_TIMEOUT_MS) {
      //No valid reading this cycle: nothing is appended
      return false;
    }
    delay(2);
  }

  // Convert RH to percent
  _rh.append((float)((((data[0] & 0x3F) << 8) + data[1]) / 16384.0) * 100.0);
  // Convert Temp
  _temp.append((float)((unsigned((data[2] * 64)) + unsigned((data[3] >> 2))) / 16384.0) * 165.0 - 40.0);
  return true;
}

uint8_t T9602::getStatus() {
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

//The summary interface: the columns a logger writes, streamed straight into the
//file. No row is composed in RAM; see LIBRARY-DESIGN.md section 14.
size_t T9602::printDataHeader(Print& out) {
  size_t n = 0;
  n += out.print("Humidity [%],");
  if (_cfg.columns()) n += out.print("Humidity std [%],Humidity sterr [%],");
  n += out.print("Temp Atmos [C],");
  if (_cfg.columns()) n += out.print("Temp Atmos std [C],Temp Atmos sterr [C],");
  return n;
}

size_t T9602::printDataRow(Print& out) {
  //The values the last updateMeasurements() left, in printDataHeader()'s
  //order. This takes no reading: the caller has already acquired, and a row
  //written to two sinks must not acquire twice.
  size_t n = 0;
  n += out.print(RH);
  n += out.print(',');
  if (_cfg.columns()) {
    n += out.print(getHumidityStd());
    n += out.print(',');
    n += out.print(getHumiditySterr());
    n += out.print(',');
  }
  n += out.print(Temp);
  n += out.print(',');
  if (_cfg.columns()) {
    n += out.print(getTemperatureStd());
    n += out.print(',');
    n += out.print(getTemperatureSterr());
    n += out.print(',');
  }
  return n;
}



//The reading interface: one reading per logReading(), printed as it is taken.
void T9602::beginReadings(uint16_t n) {
  (void)n;  //Not a Schema 1 device: nothing to declare to the sensor
  _rh.reset();
  _temp.reset();
}

void T9602::endReadings() {
  //No cleanup required currently
}

size_t T9602::printHeader(Print& out) {
  return out.print("Humidity [%],Temp Atmos [C],");
}

size_t T9602::printReading(Print& out) {
  size_t n = 0;
  n += out.print(RH);
  n += out.print(',');
  n += out.print(Temp);
  n += out.print(',');
  return n;
}

size_t T9602::logReading(Print& out) {
  if (readOnce()) {
    RH = _rh.last();
    Temp = _temp.last();
  } else {
    RH = NW_ERROR;
    Temp = NW_ERROR;
  }
  return printReading(out);
}

bool T9602::sleep() {
  //Add sleep command
  return false;
}
