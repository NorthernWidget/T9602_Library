// Minimal host-side stand-in for the Arduino core, enough to compile and run
// T9602.cpp on a desktop for output-regression testing. Not a general emulator.
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>

inline uint32_t& _millis_counter() { static uint32_t t = 0; return t; }
inline uint32_t millis() { return _millis_counter(); }
inline void delay(uint32_t ms) { _millis_counter() += ms; }

// avr-libc dtostrf(val, width, prec, buf) is sprintf("%*.*f").
inline char* dtostrf(double val, signed char width, unsigned char prec, char* buf) {
    sprintf(buf, "%*.*f", width, prec, val); return buf;
}

// Arduino String: only what T9602 uses. Float construction mirrors WString.cpp:
// dtostrf(value, decimalPlaces + 2, decimalPlaces, buf).
class String {
    std::string s;
  public:
    String() {}
    String(const char* c) : s(c) {}
    String(const std::string& c) : s(c) {}
    String(int v) : s(std::to_string(v)) {}
    String(long v) : s(std::to_string(v)) {}
    String(unsigned int v) : s(std::to_string(v)) {}
    String(unsigned long v) : s(std::to_string(v)) {}
    String(float v, unsigned char d = 2)  { char b[33]; dtostrf(v, d + 2, d, b); s = b; }
    String(double v, unsigned char d = 2) { char b[33]; dtostrf(v, d + 2, d, b); s = b; }
    String operator+(const String& o) const { return String(s + o.s); }
    String operator+(const char* o) const { return String(s + o); }
    String& operator+=(const String& o) { s += o.s; return *this; }
    String& operator+=(const char* o) { s += o; return *this; }
    const char* c_str() const { return s.c_str(); }
};
inline String operator+(const char* a, const String& b) { return String(std::string(a) + b.c_str()); }

// Print: enough for printReading(Print&) once it exists.
class Print {
  public:
    virtual size_t write(uint8_t c) = 0;
    virtual size_t write(const uint8_t* b, size_t n) { size_t k = 0; while (n--) k += write(*b++); return k; }
    size_t print(const char* c) { return write((const uint8_t*)c, strlen(c)); }
    size_t print(char c) { return write((uint8_t)c); }
    size_t print(int v) { char b[16]; sprintf(b, "%d", v); return print(b); }
    size_t print(long v) { char b[24]; sprintf(b, "%ld", v); return print(b); }
    size_t print(unsigned int v) { char b[16]; sprintf(b, "%u", v); return print(b); }
    size_t print(unsigned long v) { char b[24]; sprintf(b, "%lu", v); return print(b); }
    size_t print(const String& v) { return print(v.c_str()); }
    // Print::printFloat (AVR core): round then print integer part, '.', digits.
    size_t print(double v, int digits = 2) {
        size_t n = 0;
        if (std::isnan(v)) return print("nan");
        if (std::isinf(v)) return print("inf");
        if (v < 0.0) { n += print('-'); v = -v; }
        double rounding = 0.5; for (int i = 0; i < digits; i++) rounding /= 10.0;
        v += rounding;
        unsigned long ip = (unsigned long)v; double rem = v - (double)ip;
        n += print(ip);
        if (digits > 0) n += print('.');
        while (digits-- > 0) { rem *= 10.0; unsigned int d = (unsigned int)rem; n += print(d); rem -= d; }
        return n;
    }
    size_t println() { return print("\n"); }
};
