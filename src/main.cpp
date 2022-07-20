#include <iostream>
#include <fstream>
#include <sstream>

#include "./intel8080.hpp"

/**
 * @brief 
 * 
 */
constexpr int RAM_SIZE = 0x10000;

/**
 * @brief 
 * 
 */
std::uint8_t ram[RAM_SIZE] = {0};

/**
 * @brief 
 * 
 */
std::ifstream f("../tests/8080EXER.COM");

/**
 * @brief 
 * 
 */
std::stringstream buffer;

/**
 * @brief 
 * 
 */
std::string code;

/**
 * @brief 
 * 
 * @param port 
 * @return std::uint8_t 
 */
std::uint8_t portIn(const std::uint8_t port)
{
	return 0;
}

/**
 * @brief 
 * 
 * @param port 
 * @param data 
 */
void portOut(const std::uint8_t port, const std::uint8_t data)
{
	// Receiving on port 0x69 for printing characters to the console
	if(port == 0x69)
	{
		std::cout << data;
	}
}

/**
 * @brief 
 */
intel8080::cpu c(portIn, portOut, ram);

int main(int argc, char** argv)
{
	/*
	buffer << f.rdbuf();
	auto code = buffer.str();
	c.load(0x100, std::vector<unsigned char>(code.begin(), code.begin() + (code.length() % RAM_SIZE)));
	*/
	intel8080::loadIntelHexFile("../tests/bios.hex", ram);

	c.PC = 0x100;

	while(not c.getHalted()){
		c.step();
	}
}