#pragma once
#include "config.h"

#include <cstdint>
#include <array>
#include "config.h"
#include "R6502.h"





class Bus
{
private:
    /* data */
public:
    Bus();
    ~Bus();

    // Devices on the bus
    R6502 cpu;

    // Fake RAM 64KB (note: compiler might complain about this)
    std::array<uint8_t, 64 * 1024> ram;

    // bus read & write functions
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr, bool ReadOnly = false);

};

TEST_CASE("Bus.h Test")
{
    Bus b;

    SUBCASE("ensure ram is reset on start"){
        CHECK(b.ram[MIN_RAM_ADDR]       == 0x00);
        CHECK(b.ram[(MAX_RAM_ADDR+1)/2] == 0x01);
        CHECK(b.ram[MAX_RAM_ADDR]       == 0x00);
    }



}

