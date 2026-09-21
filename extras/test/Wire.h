// Stub TwoWire emulating a ChipCap 2 core (the T9602's sensor) on I2C, per
// Amphenol AAS-916-127 rev. G: a Measurement Request is a write to the address
// (trailing "don't care" data allowed); a Data Fetch is requestFrom(adr, n),
// returning RH[13:8] with the status bits in the two MSBs, RH[7:0], T[13:6],
// T[5:0]<<2. Status 00 = valid (not fetched since the last measurement cycle),
// 01 = stale (already fetched), 10 = command mode (start-up). A fetch marks the
// current data as fetched; a new cycle completes every cycleMs of millis().
#pragma once
#include <cstdint>
#include <deque>

class TwoWire {
  public:
    uint8_t deviceAddress = 0x28;
    bool present = true;              // false: no ACK, reads return nothing (0xFF)
    uint32_t cycleMs = 100;           // Update-mode period (0.70–165 ms per the guide)
    uint32_t readyAt = 0;             // millis() at which the first valid cycle completes (start-up)
    uint32_t lastCycle = 0;           // millis() of the last completed cycle
    bool fetched = false;             // current data already fetched -> stale
    bool cycled = true;               // false until the first measurement cycle completes
    uint16_t rh14 = 0, t14 = 0;       // current measurement (raw 14-bit)
    uint16_t nextRh14 = 0, nextT14 = 0; // what the next cycle will produce
    unsigned transactions = 0, requests = 0;

    void begin() {}
    void beginTransmission(uint8_t adr) { _adr = adr; }
    size_t write(uint8_t) { return 1; }
    uint8_t endTransmission(bool = true) {
        if (!present || _adr != deviceAddress) return 2;
        requests++; return 0;         // MR: counted; no effect in Update mode
    }
    uint8_t requestFrom(uint8_t adr, uint8_t n) {
        transactions++;
        if (!present || adr != deviceAddress) return 0;
        uint8_t status;
        if (millis() < readyAt) { status = 0x2; }                         // command mode: no data yet
        else {
            if (!cycled || millis() - lastCycle >= cycleMs) {            // a cycle completed since the last one
                lastCycle = millis() - (millis() - readyAt) % cycleMs; cycled = true;
                rh14 = nextRh14; t14 = nextT14; fetched = false;
            }
            status = fetched ? 0x1 : 0x0; fetched = true;
        }
        uint8_t f[4] = { (uint8_t)((status << 6) | ((rh14 >> 8) & 0x3F)), (uint8_t)(rh14 & 0xFF),
                         (uint8_t)(t14 >> 6), (uint8_t)((t14 & 0x3F) << 2) };
        for (uint8_t i = 0; i < n && i < 4; i++) _q.push_back(f[i]);
        return n;
    }
    int read() { if (_q.empty()) return -1; int v = _q.front(); _q.pop_front(); return v; }
    int available() { return (int)_q.size(); }
  private:
    uint8_t _adr = 0; std::deque<uint8_t> _q;
};
extern TwoWire Wire;
