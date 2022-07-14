/*
Copyright (C) 2022 Weiju Wang.

This file is part of `intel8080`.

`intel8080` is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

`intel8080` is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. 
*/

#include "./intel8080.hpp"

#include <initializer_list>
#include <limits>

using namespace intel8080;

std::uint8_t& reg::high(void) noexcept
{
	return pair[0];
}

std::uint8_t& reg::low(void) noexcept
{
	return pair[1];
}

invalidAsm::invalidAsm(const std::size_t line)
:
	lineNum(line)
{}

const char* invalidAsm::what(void) const throw()
{
	return "Unknown instruction";
}

cpu::cpu(typeof portInputHandler pih, typeof portOutputHandler poh, typeof(ram) preAllocatedRam) noexcept
:
	ram(preAllocatedRam), portInputHandler(pih), portOutputHandler(poh)
{
	setFlag(flag(1), true);
}

std::uint16_t& cpu::PSW(void) noexcept
{
	return _PSW.full;
}

std::uint16_t& cpu::BC(void) noexcept
{
	return _BC.full;
}

std::uint16_t& cpu::DE(void) noexcept
{
	return _DE.full;
}

std::uint16_t& cpu::HL(void) noexcept
{
	return _HL.full;
}

std::uint8_t& cpu::A(void) noexcept
{
	return _PSW.high();
}

std::uint8_t& cpu::flags(void) noexcept
{
	return _PSW.low();
}

std::uint8_t& cpu::B(void) noexcept
{
	return _BC.high();
}

std::uint8_t& cpu::C(void) noexcept
{
	return _BC.low();
}

std::uint8_t& cpu::D(void) noexcept
{
	return _DE.high();
}

std::uint8_t& cpu::E(void) noexcept
{
	return _DE.low();
}

std::uint8_t& cpu::H(void) noexcept
{
	return _HL.high();
}

std::uint8_t& cpu::L(void) noexcept
{
	return _HL.low();
}

std::uint8_t& cpu::atHL(void) noexcept
{
	return ram[HL()];
}

std::uint16_t& cpu::atSP(void) noexcept
{
	return *(std::uint16_t*)(ram + SP);
}

int cpu::getFlag(const flag f) noexcept
{
	return (flags() >> f) % 2;
}

void cpu::setFlag(const flag f, const bool condition) noexcept
{
	if(condition)
	{
		flags() |= (1U << f);
	}
	else
	{
		flags() &= ~(1U << f);
	}
}

void cpu::load(const std::size_t orig, const std::initializer_list<std::uint8_t>& code) noexcept
{
	int i = 0;

	for(auto byte : code)
	{
		ram[orig + i] = byte;
		++i;
	}
}

void cpu::interrupt(const std::uint8_t interruptVector)
{
	interruptsEnabled = false;
	interruptPending = true;
	this->interruptVector = interruptVector;
}

void cpu::step(void)
{
	if(interruptPending)
	{
		exec(interruptVector);
		interruptPending = false;
		halted = false;
	}
	else if(not halted)
	{
		exec(get8());
	}
}

#if INTEL8080_DEBUG__

	void cpu::dump(void) noexcept
	{
		std::cout
		<< "===============================+==========\n"
		<< "Registers                      | Flags\n"
		<< "-------------------------------+----------\n"
		<< " A  B  C  D  E  H  L   SP   PC | S Z A P C\n"
		<< std::hex
		<< std::setw(2) << (int)A() << " "
		<< std::setw(2) << (int)B() << " "
		<< std::setw(2) << (int)C() << " "
		<< std::setw(2) << (int)D() << " "
		<< std::setw(2) << (int)E() << " "
		<< std::setw(2) << (int)H() << " "
		<< std::setw(2) << (int)L() << " "
		<< std::setw(4) << SP << " "
		<< std::setw(4) << PC << " | "
		<< getFlag(flag::sign) << " "
		<< getFlag(flag::zero) << " "
		<< getFlag(flag::auxCarry) << " "
		<< getFlag(flag::parity) << " "
		<< getFlag(flag::carry)
		<< "\n===============================+==========\n";
	}

#endif

std::uint8_t cpu::get8(void) noexcept
{
	return ram[PC++];
}

std::uint16_t cpu::get16(void) noexcept
{
	auto copy = *(std::uint16_t*)(ram + PC);
	PC += 2;
	return copy;
}

template<typename T>
void cpu::updateFlags(const T result) noexcept
{
	setFlag(sign, (std::make_signed_t<T>)result < 0);
	setFlag(zero, result == 0);
	setFlag(parity, not __builtin_parity(result));
}

template<typename T>
void cpu::updateCarryFlag(const T a, const T b) noexcept
{
	setFlag(carry, b > std::numeric_limits<T>::max() - a);
}

template<typename T>
void cpu::updateAuxCarryFlag(const T a, const T b) noexcept
{
	setFlag(auxCarry, lowBitsOf(b, 4) + lowBitsOf(a, 4) > 0b1111);
}

void cpu::inr(std::uint8_t& r8) noexcept
{
	setFlag(auxCarry, lowBitsOf(r8, 4) == 0b1111);
	updateFlags(++r8);
}

void cpu::dcr(std::uint8_t& r8) noexcept
{
	setFlag(auxCarry, lowBitsOf(r8, 4) == 0b0000);
	updateFlags(--r8);
}

void cpu::dad(std::uint16_t& r16) noexcept
{
	setFlag(carry, r16 > std::numeric_limits<std::uint16_t>::max() - HL());
	HL() += r16;
}

void cpu::add(const std::uint8_t r8, const bool withCarry /* = false */) noexcept
{
	std::uint8_t added = r8;
	if(withCarry and getFlag(carry)) ++added;

	updateCarryFlag(A(), added);
	updateAuxCarryFlag(A(), added);
	updateFlags(A() += added);
}

void cpu::sub(const std::uint8_t r8, const bool withBorrow /* = false */) noexcept
{
	add(twosComp(r8), withBorrow);
}

void cpu::cmp(const std::uint8_t r8) noexcept
{
	auto copyA = A();
	sub(r8);
	setFlag(carry, not getFlag(carry));
	A() = copyA;
}

void cpu::logicAnd(const std::uint8_t r8) noexcept
{
	updateFlags(A() &= r8);
	setFlag(carry, false);
}

void cpu::logicOr(const std::uint8_t r8) noexcept
{
	updateFlags(A() |= r8);
	setFlag(carry, false);
}

void cpu::logicXor(const std::uint8_t r8) noexcept
{
	updateFlags(A() ^= r8);
	setFlag(carry, false);
}

void cpu::push(const std::uint16_t r16) noexcept
{
	SP -= 2;
	atSP() = swapBytes(r16);
}

void cpu::pop(std::uint16_t& r16) noexcept
{
	r16 = swapBytes(atSP());
	SP += 2;

	// Reset unused flags
	setFlag((flag)5, 0);
	setFlag((flag)3, 0);
	setFlag((flag)1, 1);
}

void cpu::rst(const std::uint8_t which) noexcept
{
	push(PC);
	PC = 8 * which;
}

void cpu::jmp(const bool condition) noexcept
{
	if(condition)
	{
		PC = get16();
	}
}

void cpu::ret(const bool condition) noexcept
{
	if(condition)
	{
		pop(PC);
	}
}

void cpu::call(const bool condition) noexcept
{
	if(condition)
	{
		push(PC);
		PC = get16();
	}
}

void cpu::exec(const std::uint8_t instr) noexcept
{
	std::uint16_t temp;

	switch(instr)
	{
		// NOP, incl. undocumented
		case 0x00:
		case 0x08:
		case 0x10:
		case 0x18:
		case 0x20:
		case 0x28:
		case 0x30:
		case 0x38:
			// Intentionally empty
			break;

		// LXI r16, d16
		case 0x01: BC() = get16(); break;
		case 0x11: DE() = get16(); break;
		case 0x21: HL() = get16(); break;
		case 0x31: SP = get16(); break;

		// STAX r16
		case 0x02: ram[BC()] = A(); break;
		case 0x12: ram[DE()] = A(); break;

		// LDAX r16
		case 0x0a: A() = ram[BC()]; break;
		case 0x1a: A() = ram[DE()]; break;

		// SHLD a16
		case 0x22: ram[get16()] = HL(); break;

		// LHLD a16
		case 0x2a: HL() = ram[get16()]; break;

		// STA a16
		case 0x32: ram[get16()] = A(); break;

		// LDA a16
		case 0x3a: A() = ram[get16()]; break;

		// INX r16
		case 0x03: ++BC(); break;
		case 0x13: ++DE(); break;
		case 0x23: ++HL(); break;
		case 0x33: ++SP; break;

		// DCX r16
		case 0x0b: --BC(); break;
		case 0x1b: --DE(); break;
		case 0x2b: --HL(); break;
		case 0x3b: --SP; break;

		// INR r8
		case 0x04: inr(B()); break;
		case 0x0c: inr(C()); break;
		case 0x14: inr(D()); break;
		case 0x1c: inr(E()); break;
		case 0x24: inr(H()); break;
		case 0x2c: inr(L()); break;
		case 0x34: inr(atHL()); break;
		case 0x3c: inr(A()); break;

		// DCR r8
		case 0x05: dcr(B()); break;
		case 0x0d: dcr(C()); break;
		case 0x15: dcr(D()); break;
		case 0x1d: dcr(E()); break;
		case 0x25: dcr(H()); break;
		case 0x2d: dcr(L()); break;
		case 0x35: dcr(atHL()); break;
		case 0x3d: dcr(A()); break;

		// MVI r8
		case 0x06: B() = get8(); break;
		case 0x0e: C() = get8(); break;
		case 0x16: D() = get8(); break;
		case 0x1e: E() = get8(); break;
		case 0x26: H() = get8(); break;
		case 0x2e: L() = get8(); break;
		case 0x36: atHL() = get8(); break;
		case 0x3e: A() = get8(); break;

		// RLC
		case 0x07:							// (on register A:)
			temp = highBitsOf(A(), 1);		// Extract bit 7
			setFlag(carry, temp);			// Carry flag <- bit 7
			A() <<= 1;						// Left shift 1
			A() += (std::uint8_t)temp;		// Bit 0 <- bit 7
			break;

		// RRC
		case 0x0f:									// (on register A:)
			temp = lowBitsOf(A(), 1);				// Extract bit 0
			setFlag(carry, temp);					// Carry flag <- bit 0
			A() >>= 1;								// Right shift 1
			A() += (1U << 7) * (std::uint8_t)temp;	// Bit 7 <- bit 0
			break;

		// RAL
		case 0x17: // Same as RLC, but bit 0 is not replaced with bit 7
			setFlag(carry, highBitsOf(A(), 1));
			A() <<= 1;
			break;

		// RAR
		case 0x1f: // Same as RRC, but bit 7 is not replaced with bit 0
			setFlag(carry, lowBitsOf(A(), 1));
			A() >>= 1;
			break;

		// DAA
		case 0x27:
			if(getFlag(auxCarry) or lowBitsOf(A(), 4) > 9)
			{
				A() += 6;
				setFlag(auxCarry, true);
			}

			if(getFlag(carry) or highBitsOf(A(), 4) > 9)
			{
				A() += (6U << 4);
				setFlag(carry, true);
			}

			updateFlags(A());
			break;

		// STC
		case 0x37: setFlag(carry, true); break;

		// CMA
		case 0x2f: A() = ~A(); break;

		// CMC
		case 0x3f: setFlag(carry, not getFlag(carry)); break;

		// DAD r16
		case 0x09: dad(BC()); break;
		case 0x19: dad(DE()); break;
		case 0x29: dad(HL()); break;
		case 0x39: dad(SP); break;

		// MOV r8, r8
		case 0x40: B() = B(); break;
		case 0x41: B() = C(); break;
		case 0x42: B() = D(); break;
		case 0x43: B() = E(); break;
		case 0x44: B() = H(); break;
		case 0x45: B() = L(); break;
		case 0x46: B() = atHL(); break;
		case 0x47: B() = A(); break;
		case 0x48: C() = B(); break;
		case 0x49: C() = C(); break;
		case 0x4a: C() = D(); break;
		case 0x4b: C() = E(); break;
		case 0x4c: C() = H(); break;
		case 0x4d: C() = L(); break;
		case 0x4e: C() = atHL(); break;
		case 0x4f: C() = A(); break;
		case 0x50: D() = B(); break;
		case 0x51: D() = C(); break;
		case 0x52: D() = D(); break;
		case 0x53: D() = E(); break;
		case 0x54: D() = H(); break;
		case 0x55: D() = L(); break;
		case 0x56: D() = atHL(); break;
		case 0x57: D() = A(); break;
		case 0x58: E() = B(); break;
		case 0x59: E() = C(); break;
		case 0x5a: E() = D(); break;
		case 0x5b: E() = E(); break;
		case 0x5c: E() = H(); break;
		case 0x5d: E() = L(); break;
		case 0x5e: E() = atHL(); break;
		case 0x5f: E() = A(); break;
		case 0x60: H() = B(); break;
		case 0x61: H() = C(); break;
		case 0x62: H() = D(); break;
		case 0x63: H() = E(); break;
		case 0x64: H() = H(); break;
		case 0x65: H() = L(); break;
		case 0x66: H() = atHL(); break;
		case 0x67: H() = A(); break;
		case 0x68: L() = B(); break;
		case 0x69: L() = C(); break;
		case 0x6a: L() = D(); break;
		case 0x6b: L() = E(); break;
		case 0x6c: L() = H(); break;
		case 0x6d: L() = L(); break;
		case 0x6e: L() = atHL(); break;
		case 0x6f: L() = A(); break;
		case 0x70: atHL() = B(); break;
		case 0x71: atHL() = C(); break;
		case 0x72: atHL() = D(); break;
		case 0x73: atHL() = E(); break;
		case 0x74: atHL() = H(); break;
		case 0x75: atHL() = L(); break;
		// 0x76 is used for HLT
		case 0x77: atHL() = A(); break;
		case 0x78: A() = B(); break;
		case 0x79: A() = C(); break;
		case 0x7a: A() = D(); break;
		case 0x7b: A() = E(); break;
		case 0x7c: A() = H(); break;
		case 0x7d: A() = L(); break;
		case 0x7e: A() = atHL(); break;
		case 0x7f: A() = A(); break;

		// HLT
		case 0x76: halted = true; break;

		// ADD r8
		case 0x80: add(B()); break;
		case 0x81: add(C()); break;
		case 0x82: add(D()); break;
		case 0x83: add(E()); break;
		case 0x84: add(H()); break;
		case 0x85: add(L()); break;
		case 0x86: add(atHL()); break;
		case 0x87: add(A()); break;

		// ADC r8
		case 0x88: add(B(), true); break;
		case 0x89: add(C(), true); break;
		case 0x8a: add(D(), true); break;
		case 0x8b: add(E(), true); break;
		case 0x8c: add(H(), true); break;
		case 0x8d: add(L(), true); break;
		case 0x8e: add(atHL(), true); break;
		case 0x8f: add(A(), true); break;

		// SUB r8
		case 0x90: sub(B()); break;
		case 0x91: sub(C()); break;
		case 0x92: sub(D()); break;
		case 0x93: sub(E()); break;
		case 0x94: sub(H()); break;
		case 0x95: sub(L()); break;
		case 0x96: sub(atHL()); break;
		case 0x97: sub(A()); break;

		// SBB r8
		case 0x98: sub(B(), true); break;
		case 0x99: sub(C(), true); break;
		case 0x9a: sub(D(), true); break;
		case 0x9b: sub(E(), true); break;
		case 0x9c: sub(H(), true); break;
		case 0x9d: sub(L(), true); break;
		case 0x9e: sub(atHL(), true); break;
		case 0x9f: sub(A(), true); break;

		// ANA r8
		case 0xa0: logicAnd(B()); break;
		case 0xa1: logicAnd(C()); break;
		case 0xa2: logicAnd(D()); break;
		case 0xa3: logicAnd(E()); break;
		case 0xa4: logicAnd(H()); break;
		case 0xa5: logicAnd(L()); break;
		case 0xa6: logicAnd(atHL()); break;
		case 0xa7: logicAnd(A()); break;

		// XRA r8
		case 0xa8: logicXor(B()); break;
		case 0xa9: logicXor(C()); break;
		case 0xaa: logicXor(D()); break;
		case 0xab: logicXor(E()); break;
		case 0xac: logicXor(H()); break;
		case 0xad: logicXor(L()); break;
		case 0xae: logicXor(atHL()); break;
		case 0xaf: logicXor(A()); break;

		// ORA r8
		case 0xb0: logicOr(B()); break;
		case 0xb1: logicOr(C()); break;
		case 0xb2: logicOr(D()); break;
		case 0xb3: logicOr(E()); break;
		case 0xb4: logicOr(H()); break;
		case 0xb5: logicOr(L()); break;
		case 0xb6: logicOr(atHL()); break;
		case 0xb7: logicOr(A()); break;

		// CMP r8
		case 0xb8: cmp(B()); break;
		case 0xb9: cmp(C()); break;
		case 0xba: cmp(D()); break;
		case 0xbb: cmp(E()); break;
		case 0xbc: cmp(H()); break;
		case 0xbd: cmp(L()); break;
		case 0xbe: cmp(atHL()); break;
		case 0xbf: cmp(A()); break;

		// ADI
		case 0xc6: add(get8()); break;

		// ACI
		case 0xce: add(get8(), true); break;

		// SUI
		case 0xd6: sub(get8()); break;

		// SBI
		case 0xde: sub(get8(), true); break;

		// ANI
		case 0xe6: logicAnd(get8()); break;

		// XRI
		case 0xee: logicXor(get8()); break;

		// ORI
		case 0xf6: logicOr(get8()); break;

		// CPI
		case 0xfe: cmp(get8()); break;

		// XCHG
		case 0xeb:
			temp = HL();
			HL() = DE();
			DE() = temp;
			break;

		// XTHL
		case 0xe3:
			temp = swapBytes(*(std::uint16_t*)(ram + SP));
			*(std::uint16_t*)(ram + SP) = swapBytes(HL());
			HL() = temp;
			break;

		// SPHL
		case 0xf9: SP = swapBytes(HL()); break;

		// PCHL
		case 0xe9: PC = swapBytes(HL()); break;

		// DI
		case 0xf3: interruptsEnabled = false; break;

		// EI
		case 0xfb: interruptsEnabled = true; break;

		// PUSH r16
		case 0xc5: push(BC()); break;
		case 0xd5: push(DE()); break;
		case 0xe5: push(HL()); break;
		case 0xf5: push(PSW()); break;

		// POP r16
		case 0xc1: pop(BC()); break;
		case 0xd1: pop(DE()); break;
		case 0xe1: pop(HL()); break;
		case 0xf1: pop(PSW()); break;

		// IN p8
		case 0xdb: A() = portInputHandler(get8()); break;

		// OUT p8
		case 0xd3: portOutputHandler(get8(), A()); break;

		// RST
		case 0xc7: rst(0); break;
		case 0xcf: rst(1); break;
		case 0xd7: rst(2); break;
		case 0xdf: rst(3); break;
		case 0xe7: rst(4); break;
		case 0xef: rst(5); break;
		case 0xf7: rst(6); break;
		case 0xff: rst(7); break;

		// JNZ a16
		case 0xc2: jmp(not getFlag(zero)); break;

		// JMP a16
		case 0xc3: jmp(true); break;

		// JZ a16
		case 0xca: jmp(getFlag(zero)); break;

		// JNC a16
		case 0xd2: jmp(not getFlag(carry)); break;

		// JC a16
		case 0xda: jmp(getFlag(carry)); break;

		// JPO a16
		case 0xe2: jmp(not getFlag(parity)); break;

		// JPE a16
		case 0xea: jmp(getFlag(parity)); break;

		// JP a16
		case 0xf2: jmp(not getFlag(sign)); break;

		// JM a16
		case 0xfa: jmp(getFlag(sign)); break;

		// RET, incl. undocumented
		case 0xc9:
		case 0xd9: ret(true); break;

		// RNZ
		case 0xc0: ret(not getFlag(zero)); break;

		// RZ
		case 0xc8: ret(getFlag(zero)); break;

		// RNC
		case 0xd0: ret(not getFlag(carry)); break;

		// RC
		case 0xd8: ret(getFlag(carry)); break;

		// RPO
		case 0xe0: ret(not getFlag(parity)); break;

		// RPE
		case 0xe8: ret(getFlag(parity)); break;

		// RP
		case 0xf0: ret(not getFlag(sign)); break;

		// RM
		case 0xf8: ret(getFlag(sign)); break;

		// CALL, incl. undocumented
		case 0xcd:
		case 0xdd:
		case 0xed:
		case 0xfd: call(true); break;

		// CNZ
		case 0xc4: call(not getFlag(zero)); break;

		// CZ
		case 0xcc: call(getFlag(zero)); break;

		// CNC
		case 0xd4: call(not getFlag(carry)); break;

		// CC
		case 0xdc: call(getFlag(carry)); break;

		// CPO
		case 0xe4: call(not getFlag(parity)); break;

		// CPE
		case 0xec: call(getFlag(parity)); break;

		// CP
		case 0xf4: call(not getFlag(sign)); break;

		// CM
		case 0xfc: call(getFlag(sign)); break;

		// All cases should be covered; no `default` is necessary.
	}
}

std::uint16_t intel8080::swapBytes(const std::uint16_t n)
{
	return highBitsOf(n, 8) + lowBitsOf(n, 8) * (1U << 8);
}

std::vector<std::uint8_t> intel8080::assemble(const std::string& code)
{
	// TODO intel8080::assemble()
	return {};
}