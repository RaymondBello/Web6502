#include "config.h"
#include "Bus.h"

Bus::Bus()
{
    // Reset RAM content
    for (auto &i : ram) i = 0x00;

    // Connect cpu to BUS
    cpu.ConnectBus(this);
}

Bus::~Bus()
{
}

/**
 * @brief write data to a specified address
 * 
 * @param addr address to be written to
 * @param data data to be written
 */
void Bus::write(uint16_t addr, uint8_t data)
{
    if (addr >= MIN_RAM_ADDR && addr <= MAX_RAM_ADDR) 
        ram[addr] = data;
}

/**
 * @brief read data from a  specified address, returns data
 * In normal operation "Read Only" is set to false.
 * Some devices on the bus may change state when they are read from, and this 
 * is intentional under normal circumstances. However the disassembler will
 * want to read the data at an address without changing the state of the
 * devices on the bus
 * 
 * @param addr address to be read from
 * @param ReadOnly true if only read is needed, default is false
 * @return uint8_t 
 */
uint8_t Bus::read(uint16_t addr, bool ReadOnly)
{
    if (addr >= MIN_RAM_ADDR && addr <= MAX_RAM_ADDR)
        return ram[addr];
    return 0x00;
}













