#include <iostream>
#include <fstream>
#include <string>

#include "intel8080/intel8080.hpp"

std::uint8_t ram[0x10000] = {0};

std::uint8_t portIn(const std::uint8_t port)
{
	return 0;
}

void portOut(const std::uint8_t port, const std::uint8_t data)
{

}

intel8080::cpu c(portIn, portOut, ram);

template<typename T>
inline void print(const T s) noexcept
{
	std::cout << std::to_string(s) << std::endl;
}

int main(int argc, char** argv)
{
	c.load(0, { 0x01, 0x34, 0x12 });
	c.step();
	c.dump();
}