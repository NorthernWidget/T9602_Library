// Output-regression test for T9602_Library: compiles src/T9602.cpp against the
// stubs here and prints what the library reports for fixed ChipCap 2 behaviour.
// run.sh diffs the result against baseline.txt.
#include "Arduino.h"
#include "Wire.h"
TwoWire Wire;
#include "../../src/T9602.cpp"

static uint16_t rhRaw(float pct) { return (uint16_t)(pct / 100.0f * 16384.0f + 0.5f); }
static uint16_t tRaw(float degC) { return (uint16_t)((degC + 40.0f) / 165.0f * 16384.0f + 0.5f); }

// The ChipCap 2 core emulated on NW_Core's register-image stub: the sensor runs
// its own measurement cycle (Update mode), the status bits in the first byte say
// valid (00), stale (01, already fetched since the last cycle) or command mode
// (10, start-up); the four bytes are rebuilt before every read (beforeRead).
struct ChipCap {
    uint32_t cycleMs = 100;            // Update-mode period (0.70-165 ms per the guide)
    uint32_t readyAt = 0;              // millis() at which the first valid cycle completes (start-up)
    uint32_t lastCycle = 0;            // millis() of the last completed cycle
    bool fetched = false;              // current data already fetched -> stale
    bool cycled = true;                // false until the first measurement cycle completes
    uint16_t rh14 = 0, t14 = 0;        // current measurement (raw 14-bit)
    uint16_t nextRh14 = 0, nextT14 = 0; // what the next cycle will produce
} chip;
static void installChipCap() {
    Wire.onRequest = [](TwoWire&, uint8_t n, std::deque<uint8_t>& out) {
        uint8_t status;
        if (millis() < chip.readyAt) { status = 0x2; }                        // command mode: no data yet
        else {
            if (!chip.cycled || millis() - chip.lastCycle >= chip.cycleMs) { // a cycle completed since the last one
                chip.lastCycle = millis() - (millis() - chip.readyAt) % chip.cycleMs; chip.cycled = true;
                chip.rh14 = chip.nextRh14; chip.t14 = chip.nextT14; chip.fetched = false;
            }
            status = chip.fetched ? 0x1 : 0x0; chip.fetched = true;
        }
        uint8_t f[4] = { (uint8_t)((status << 6) | ((chip.rh14 >> 8) & 0x3F)), (uint8_t)(chip.rh14 & 0xFF),
                         (uint8_t)(chip.t14 >> 6), (uint8_t)((chip.t14 & 0x3F) << 2) };
        for (uint8_t i = 0; i < n && i < 4; i++) out.push_back(f[i]);   // the same four bytes on every fetch: no register pointer
    };
}

// Sensor already measured (rh0, t0) and that data was fetched; the next cycle yields (rh1, t1).
static void sensor(float rh0, float t0, float rh1, float t1, uint32_t cycleMs, uint32_t readyAt, bool fetched) {
    Wire = TwoWire(); Wire.deviceAddress = 0x28; installChipCap();
    chip = ChipCap();
    chip.rh14 = rhRaw(rh0); chip.t14 = tRaw(t0); chip.nextRh14 = rhRaw(rh1); chip.nextT14 = tRaw(t1);
    chip.cycleMs = cycleMs; chip.readyAt = readyAt; chip.lastCycle = readyAt; chip.fetched = fetched; chip.cycled = (readyAt == 0);
    _millis_counter() = 0;
}

static void report(const char* name, T9602& s) {
    uint32_t t0 = millis(); unsigned n0 = Wire.transactions;
    bool ok = s.updateMeasurements();
    printf("[%s] ok=%d status=%u, took %u ms, %u fetches: %s\n", name, ok, s.getStatus(),
           (unsigned)(millis() - t0), Wire.transactions - n0, s.getString().c_str());
}

int main() {
    T9602 s; Wire.deviceAddress = 0x28;
    printf("begin: %d\n", s.begin());
    printf("header: %s\n", s.getHeader().c_str());

    // 1. Fresh data waiting: status 00 on the first fetch.
    sensor(40.0f, 20.0f, 50.0f, 25.0f, 100, 0, false); chip.rh14 = rhRaw(50.0f); chip.t14 = tRaw(25.0f);
    report("valid on first fetch", s);

    // 2. Called again before a new cycle: first fetch is stale (01) and repeats 50/25; the next cycle (at 100 ms) gives 60/30.
    sensor(50.0f, 25.0f, 60.0f, 30.0f, 100, 0, true);
    report("stale first, valid at 100 ms", s);

    // 3. Start-up: command mode (10) for 55 ms, then the first cycle (55/28).
    sensor(0.0f, -40.0f, 55.0f, 28.0f, 100, 55, false);
    report("command mode for 55 ms", s);

    // 4. Sensor absent: no ACK, reads return 0xFF.
    sensor(0, 0, 0, 0, 100, 0, false); Wire.present = false;
    printf("begin with no sensor: %d\n", s.begin());
    report("absent", s);

    // 5. getString(true) must take a new reading.
    sensor(50.0f, 25.0f, 70.0f, 35.0f, 100, 0, true); delay(100);
    printf("[getString(true)] %s\n", s.getString(true).c_str());
    printf("[getString(false)] %s\n", s.getString(false).c_str());
    return 0;
}
