/**
 * @file post.cpp
 * @author Weiju Wang (weijuwang@aol.com)
 * @brief Tests all instructions in the Intel 8080 emulator.
 * @version 0.1
 * @date 2022-07-13
 * 
 * @copyright Copyright (c) 2022 Weiju Wang.
 */

#include <iostream>
#include <fstream>

#include "./intel8080.hpp"

std::uint8_t ram[0x10000] = {0};

std::uint8_t portIn(const std::uint8_t port)
{
	return 0x69;
}

void portOut(const std::uint8_t port, const std::uint8_t data)
{
	
}

intel8080::cpu cpu(portIn, portOut, ram);

void resetAndLoad(const std::uint16_t origin, const std::vector<std::uint8_t>& code) noexcept
{
	cpu.PSW() = 0;
	cpu.BC() = 0;
	cpu.DE() = 0;
	cpu.HL() = 0;
	cpu.SP = 0;
	cpu.PC = origin;

	cpu.load(origin, code);
}

/**
 * @brief Entry point. Program execution starts here.
 * 
 * @param argc `int` Number of arguments, including the program name.
 * @param argv `char**` Arguments passed to the program, including the program name.
 * @return `int` Exit code.
 */
int main(int argc, char** argv)
{
	// NOP and all undocumented opcodes are not tested.

	resetAndLoad(0, {
		0x2a, 0x5b, 0x02,
	});

	cpu.load(0x25b, { 0xff, 0x03 });

	cpu.step();
	cpu.dump();
}