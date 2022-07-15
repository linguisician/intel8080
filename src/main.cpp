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
	buffer << f.rdbuf();
	auto code = buffer.str();
	c.load(0x100, std::vector<unsigned char>(code.begin(), code.begin() + (code.length() % RAM_SIZE)));

	// Warm boot
	c.load(0x0, {
		0x76 				// 0x0000	hlt
	});

	// BDOS character and string output
	c.load(0x5, {
		0x79,				// 0x0005	mov a, c
		0xfe, 2,			// 0x0006 	cpi 2
		0xc2, 0x0f, 0x00,	// 0x0008 	jnz 000fh
		0x7b, 				// 0x000b		mov a, e
		0xd3, 0x69,			// 0x000c		out 69h
		0xc9,				// 0x000e		ret
		0xfe, 9,			// 0x000f	cpi 9
		0xc0,				// 0x0011	rnz
		0x1a,				// 0x0012		ldax de
		0xfe, '$',			// 0x0013		cpi '$'
		0xc8,				// 0x0015		rz
		0xd3, 0x69,			// 0x0016		out 69h
		0x13,				// 0x0018		inx de
		0xc3, 0x12, 0x00,	// 0x0019		jmp 0012h
	});

	// Origin
	c.PC = 0x100;

	std::string _;

	while(not c.getHalted()){
		//std::getline(std::cin, _);
		c.step();
		//c.dump();
	}
}