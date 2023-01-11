#pragma once

#include "config.h"
#include "Bus.h"

#include <cstdint>
#include <string>
#include <vector>
#include <map>


class Bus;

class R6502
{
public:
    R6502();
    ~R6502();

public:
    // R6502 Core registers
    uint8_t a       = 0x00;   // Accumulator Register
    uint8_t x       = 0x00;   // X Register
    uint8_t y       = 0x00;   // Y Register
    uint8_t stkp    = 0x00;   // Stack Pointer (points to location on bus)
    uint8_t status  = 0x00;   // Status Register
    uint16_t pc     = 0x0000; // Program Counter

    // Assistive variables to facilitate emulation
    uint8_t fetched = 0x00;     // Represents the working input value to the ALU
    uint8_t opcode = 0x00;      // Is the instruction byte
    uint8_t cycles = 0;         // Counts how many cycles the instruction has remaining
    uint16_t temp = 0x0000;     // A convenience variable used everywhere
    uint16_t addr_abs = 0x0000; // All used memory addresses end up in here
    uint16_t addr_rel = 0x00;   // Represents absolute address following a branch
    uint32_t clock_count = 0;   // A global accumulation of the number of clocks

    // The read location of data can come from two sources, a memory address, or
    // its immediately available as part of the instruction. This function decides
    // depending on address mode of instruction byte
    uint8_t fetch();

    // External 6502 Input signals. 
    void reset(); // Reset Interrupt - Forces CPU into known state
    void irq();   // Interrupt Request - Executes an instruction at a specific location
    void nmi();   // Non-Maskable Interrupt Request - As above, but cannot be disabled
    void clock(); // Perform one clock cycle's worth of update

    // Indicates the current instruction has completed by returning true.
    // For step-by-step execution
    bool complete();

    // Produces a map of strings, with keys equivalent to instruction start locations
    // in memory, for the specified address range
    std::map<uint16_t, std::string> disassemble(uint16_t nStart, uint16_t nStop);

    // Link this CPU to a communications bus
    void ConnectBus(Bus *n) { bus = n; }

    // The status register stores 8 flags
    enum FLAGS6502
    {
        /**
         * @brief Carry Bit
         */
        C = (1 << 0),
        /**
         * @brief Zero
         */
        Z = (1 << 1),
        /**
         * @brief Disable Interrupts
         */
        I = (1 << 2),
        /**
         * @brief Decimal Mode (unused in this implementation)
         */
        D = (1 << 3),
        /**
         * @brief Break 
         */
        B = (1 << 4),
        /**
         * @brief Unused
         */
        U = (1 << 5),
        /**
         * @brief Overflow
         */
        V = (1 << 6),
        /**
         * @brief Negative
         */
        N = (1 << 7),
    };

    // 12 Addressing Modes. These functions
    // may adjust the number of cycles required depending upon where
    // and how the memory is accessed, so they return the required
    // adjustment.
    uint8_t IMP();	uint8_t IMM();	
	uint8_t ZP0();	uint8_t ZPX();	
	uint8_t ZPY();	uint8_t REL();
	uint8_t ABS();	uint8_t ABX();	
	uint8_t ABY();	uint8_t IND();	
	uint8_t IZX();	uint8_t IZY();

    // 56 opcodes. These functions return 0 normally, but some are capable of
    // requiring more clock cycles when executed under certain
    // conditions combined with certain addressing modes. If that is
    // the case, they return 1.
    uint8_t ADC();	uint8_t AND();	uint8_t ASL();	uint8_t BCC();
	uint8_t BCS();	uint8_t BEQ();	uint8_t BIT();	uint8_t BMI();
	uint8_t BNE();	uint8_t BPL();	uint8_t BRK();	uint8_t BVC();
	uint8_t BVS();	uint8_t CLC();	uint8_t CLD();	uint8_t CLI();
	uint8_t CLV();	uint8_t CMP();	uint8_t CPX();	uint8_t CPY();
	uint8_t DEC();	uint8_t DEX();	uint8_t DEY();	uint8_t EOR();
	uint8_t INC();	uint8_t INX();	uint8_t INY();	uint8_t JMP();
	uint8_t JSR();	uint8_t LDA();	uint8_t LDX();	uint8_t LDY();
	uint8_t LSR();	uint8_t NOP();	uint8_t ORA();	uint8_t PHA();
	uint8_t PHP();	uint8_t PLA();	uint8_t PLP();	uint8_t ROL();
	uint8_t ROR();	uint8_t RTI();	uint8_t RTS();	uint8_t SBC();
	uint8_t SEC();	uint8_t SED();	uint8_t SEI();	uint8_t STA();
	uint8_t STX();	uint8_t STY();	uint8_t TAX();	uint8_t TAY();
	uint8_t TSX();	uint8_t TXA();	uint8_t TXS();	uint8_t TYA();

	// Invalid opcodes are caught here. Functionally identical to a NOP
	uint8_t XXX();

private:
    Bus *bus = nullptr;
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);

    // functions to access status register
    uint8_t GetFlag(FLAGS6502 f);
    void SetFlag(FLAGS6502 f, bool value);

    /**
     * @brief This structure and the following vector are used to compile and store
     * the opcode translation table. The 6502 can effectively have 256
     * different instructions. Each of these are stored in a table in numerical
     * order so they can be looked up easily, with no decoding required.
     * Each table entry holds:
     *      Pneumonic : A textual representation of the instruction (used for disassembly)
     *  	Opcode Function: A function pointer to the implementation of the opcode
     *  	Opcode Address Mode : A function pointer to the implementation of the 
     *  						  addressing mechanism used by the instruction
     *  	Cycle Count : An integer that represents the base number of clock cycles the
     *  				  CPU requires to perform the instruction 
     */
    struct INSTRUCTION
    {
        std::string name;
        uint8_t (R6502::*operate)(void) = nullptr;
        uint8_t (R6502::*addrmode)(void) = nullptr;
        uint8_t cycles = 0;
    };

    std::vector<INSTRUCTION> lookup;

#ifdef LOG_MODE
private:
    FILE *logfile = nullptr;
#endif
};



TEST_CASE("R6502.h tests")
{
    CHECK(1 == true);
}

