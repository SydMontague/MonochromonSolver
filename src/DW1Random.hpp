#pragma once

#include <cstdint>

class DW1Random
{
private:
    static constexpr uint32_t multiplier = 0x41C64E6D;
    static constexpr uint32_t increment  = 0x3039;
    uint32_t state;

public:
    DW1Random(uint32_t seed)
        : state(seed)
    {
    }

    uint16_t next()
    {
        state = state * multiplier + increment;
        return (state >> 0x10) & 0x7FFF;
    }

    void advance(uint32_t amount)
    {
        for (uint32_t i = 0; i < amount; i++)
            next();
    }

    uint32_t next(uint32_t limit) { return (next() * limit) >> 0xF; }

    uint32_t nextModulo(uint32_t limit) { return next() % limit; }

    uint32_t getState() { return state; }

    void setState(uint32_t new_state) {  state = new_state; }
};