/**
 * @file intel8080.hpp
 * @author Weiju Wang (weijuwang@aol.com)
 * @brief An emulator for the Intel 8080 microprocessor.
 * @version 0.1
 * @date 2022-07-13
 * 
 * @copyright Copyright (c) 2022 Weiju Wang.
 * This file is part of `intel8080`.
 * `intel8080` is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * `intel8080` is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. 
 */

#pragma once

#define INTEL8080_DEBUG__ true

#include <set>
#include <vector>
#include <cinttypes>
#include <functional>

#if INTEL8080_DEBUG__
	#include <iostream>
	#include <iomanip>
#endif

/**
 * @brief Defines types and utility functions for emulating the Intel 8080 microprocessor.
 * When in doubt, refer to the Wikipedia page and the Programmer's Manual
   (linked below) for details about how the 8080 works, as this documentation
   assumes at least partial knowledge thereof.
 * @see https://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf
 * @see https://en.wikipedia.org/wiki/Intel_8080
 * @see https://pastraiser.com/cpu/i8080/i8080_opcodes.html
 * 
 * @note The Intel 8080 is little-endian, meaning bits with higher place values
   are stored in lower addresses. When a 16-bit integer in memory is represented
   as two bytes, it will appear "backwards"; e.g. `0x1234` is stored as [`0x34`,
   `0x12`]. In this code, `high bits` refer to bits at higher place values; vice
   versa with `low bits`. @see https://en.wikipedia.org/wiki/Endianness
 */
namespace intel8080
{
	/**
	 * @brief An 8-bit unsigned integer.
	 */
	using byte = std::uint8_t;

	/**
	 * @brief A 16-bit unsigned integer.
	 */
	using bytePair = std::uint16_t;

	/**
	 * @brief Positions of CPU flags.
	 * @note Flags 5, 3, and 1 are unused.
	 * @note Flags 5 and 3 are always 0; flag 1 is always 1.
	 */
	enum flagPos
	{
		sign = 7,		// Set if the result is negative (two's complement).
		zero = 6,		// Set if the result is zero.
		auxCarry = 4,	// Set if there was a carry from bit 3.
		parity = 2,		// Set if the result has an even number of set bits.
		carry = 0		// Set if there was a carry (from bit 7).
	};

	/**
	 * @brief A pair of 8-bit registers.
	 */
	union regPair
	{
		/**
		 * @brief A pair of 8-bit registers, each represented independently.
		 */
		byte pair[2];

		/**
		 * @brief A pair of 8-bit registers, represented as one 16-bit register.
		 */
		bytePair full;

		/**
		 * @brief Equivalent to `pair[0]`.
		 * @return `byte&` A mutable reference to the low byte of the pair.
		 */
		byte& low(void) noexcept;

		/**
		 * @brief Equivalent to `pair[1]`.
		 * @return `byte&` A mutable reference to the high byte of the pair.
		 */
		byte& high(void) noexcept;
	};

	/**
	 * @brief Represents an individual Intel 8080.
	 * The built-in public interface only allows the user to run one instruction
	   at a time, using `step(void)`. This is to allow for flexible
	   implementation of debugging and interrupts. The user is responsible for
	   combining public functions to allow the CPU to run continuously, or
	   in whatever manner is required. 
	 */
	class cpu
	{
	public:
		/**
		 * @brief Stack pointer.
		 */
		bytePair SP;

		/**
		 * @brief Program counter (the address in memory from which the next
		   instruction should be fetched).
		 */
		bytePair PC = 0;

		/**
		 * @brief A pointer to space in memory used as RAM.
		 */
		byte* ram;

		/**
		 * @brief Handles data outputted to ports by the `out` instruction.
		 * @param port `const byte` The port number.
		 * @param data `const byte` The data that the port is receiving.
		 */
		std::function<void(const byte port, const byte data)> portOutputHandler;

		/**
		 * @brief Provides data for port input requests from the `in` instruction.
		 * @param port `const byte` The port number.
		 * @return `byte` The byte that the port received.
		 */
		std::function<byte(const byte port)> portInputHandler;

		/**
		 * @brief Construct a new Intel 8080.
		 * 
		 * @param portInputHandler The function that is called when a port input
		   is needed.
		 * @param portOutputHandler The function that is called when data is
		   outputted to a port.
		 * @param ram `byte* = nullptr` A pointer to 65536 bytes in memory used
		   as RAM. The user is responsible for freeing this memory.
		 *
		 * @see `portInputHandler`
		 * @see `portOutputHandler`
		 */
		cpu(typeof portInputHandler, typeof portOutputHandler, typeof ram = nullptr) noexcept;

		/**
		 * @return `bytePair&` A mutable reference to the program state word
		   (accumulator and flags).
		 */
		bytePair& PSW(void) noexcept;

		/**
		 * @return `bytePair&` A mutable reference to the register pair BC.
		 */
		bytePair& BC(void) noexcept;

		/**
		 * @return `bytePair&` A mutable reference to the register pair DE.
		 */
		bytePair& DE(void) noexcept;

		/**
		 * @return `bytePair&` A mutable reference to the register pair HL.
		 */
		bytePair& HL(void) noexcept;

		/**
		 * @return `byte&` A mutable reference to the accumulator.
		 */
		byte& A(void) noexcept;

		/**
		 * @return `byte&` A mutable reference to the flags register.
		 */
		byte& flags(void) noexcept;

		/**
		 * @return `byte&` A mutable reference to register B.
		 */
		byte& B(void) noexcept;

		/**
		 * @return `byte&` A mutable reference to register C.
		 */
		byte& C(void) noexcept;

		/**
		 * @return `byte&` A mutable reference to register D.
		 */
		byte& D(void) noexcept;

		/**
		 * @return `byte&` A mutable reference to register E.
		 */
		byte& E(void) noexcept;

		/**
		 * @return `byte&` A mutable reference to register H.
		 */
		byte& H(void) noexcept;

		/**
		 * @return `byte&` A mutable reference to register L.
		 */
		byte& L(void) noexcept;

		/**
		 * @return `byte&` A mutable reference to the byte pointed to by the
		   register pair HL.
		 */
		byte& atHL(void) noexcept;

		/**
		 * @return `bytePair&` A mutable reference to the 2 bytes pointed to by
		   the register pair HL.
		 */
		bytePair& atSP(void) noexcept;

		/**
		 * @param f `const flagPos` The position of the flag to retrieve.
		 * @return `int` The value of the flag at `flagPos`.
		 */
		int getFlag(const flagPos f) noexcept;

		/**
		 * @brief Sets the value of the flag at position `f` to `condition`.
		 * 
		 * @param f `const flagPos` The position of the flag to set.
		 * @param condition `bool` The value to set the flag to.
		 */
		void setFlag(const flagPos f, const bool condition) noexcept;

		/**
		 * @brief Loads raw bytes (be it data, a program, or both) to memory,
		   typically an assembled program.
		 * @param origin `const bytePair` The address in RAM to load the data to.
		 * @param bytes `const std::initializer_list<byte>&` The bytes to load. 
		 */
		void load(const bytePair origin, const std::initializer_list<byte>& bytes) noexcept;

		/**
		 * @brief Interrupts the CPU and prepares it to run the interrupt vector.
		 * @note This does not actually run the interrupt vector, but stores it
		   for later use. `step(void)` must be called for it to be run, and it
		   must be called again to execute the first instruction in memory after
		   the interrupt vector.
		 * @note Nothing happens if `interruptsEnabled` is `false`.
		 * @note See the Intel 8080 Programmer's Manual for details on how
		   interrupts work.

		 * @param instr `const byte` The interrupt vector; that is, the
		   instruction to execute after the interrupt. This is not stored in memory.
		 */
		void interrupt(const byte instr) noexcept;

		/**
		 * @brief Runs the next instruction, either the next in memory or the
		   interrupt vector if applicable.
		 * @note If `interruptPending` and `interruptsEnabled` are both true,
		   the interrupt vector `interruptVector` is run INSTEAD OF the next
		   instruction in memory.
		 */
		void step(void) noexcept;

		#if INTEL8080_DEBUG__
			/**
			 * @brief Dumps the status of all registers and flags to the console.
			 */
			void dump(void) noexcept;
		#endif

	private:

		/**
		 * @brief The program state word (accumulator and flag register).
		 */
		regPair _PSW;
		
		/**
		 * @brief The register pair BC.
		 */
		regPair _BC;

		/**
		 * @brief The register pair DE.
		 */
		regPair _DE;

		/**
		 * @brief The register pair HL.
		 */
		regPair _HL;

		/**
		 * @brief If `true`, allows interrupts to be serviced.
		 * If this is `false`, `interrupt(const byte)` will do nothing.
		 */
		bool interruptsEnabled = false;

		/**
		 * @brief If `true`, indicates that an interrupt is now pending.
		 * If this is `true`, `step(void)` runs the interrupt vector `interruptVector`.
		 */
		bool interruptPending = false;

		/**
		 * @brief The instruction to run for the pending interrupt, if any.
		 */
		byte interruptVector;

		/**
		 * @brief If `true`, the CPU will not run.
		 * That is, `step(void)` will do nothing except for servicing any
		   pending interrupts, after which `halted` will be reset to `false`.
		 */
		bool halted;

		/**
		 * @brief Returns the byte at the program counter, then increments the
		 * program counter.
		 *
		 * @return `byte` The byte previously pointed to by the program counter.
		 */
		byte get8(void) noexcept;

		/**
		 * @brief Returns the 2 bytes at the program counter, then increments
		 * the program counter.
		 *
		 * @return `byte` The 2 bytes previously pointed to by the program counter.
		 */
		bytePair get16(void) noexcept;

		/**
		 * @brief Updates the sign, zero, and parity flags based on a result.
		 * 
		 * @tparam T The type of result, either `byte` or `bytePair`.
		 * @param result `T` The result.
		 */
		template<typename T>
		void updateFlags(const T result) noexcept;

		/**
		 * @brief Updates the carry flag based on the sum of two numbers.
		 * That is, if `a + b` is greater than the maximum value `T` can hold,
		   a carry must be required in order to store the sum, and so the carry
		   flag is set to 1.
		 *
		 * @tparam T The type of numbers, either `byte` or `bytePair`.
		 * @param a `T` The first number.
		 * @param b `T` The second number.
		 */
		template<typename T>
		void updateCarryFlag(const T a, const T b) noexcept;

		/**
		 * @brief Updates the carry flag based on the sum of the lower 4 bits of
		   two numbers.
		 * That is, if the sum of the lower 4 bits of `a` and `b` is greater
		   than the maximum value a 4-bit integer can hold, an auxiliary carry
		   must be required in order to store the sum, and so the auxiliary
		   carry flag is set to 1.
		 *
		 * @param a `byte` The first number.
		 * @param b `byte` The second number.
		 */
		void updateAuxCarryFlag(const byte a, const byte b) noexcept;

		/**
		 * @brief Executes the `inr` instruction with operand `r8`.
		 * 
		 * @param r8 `byte` The register to increment.
		 */
		void inr(byte& r8) noexcept;

		/**
		 * @brief Executes the `dcr` instruction with operand `r8`.
		 * 
		 * @param r8 `byte` The register to decrement.
		 */
		void dcr(byte& r8) noexcept;

		/**
		 * @brief Executes the `dad` instruction with operand `r16`.
		 * 
		 * @param r8 `byte` The register to operate on.
		 */
		void dad(bytePair& r16) noexcept;

		/**
		 * @brief Executes the `add`, `adc`, `aci`, or `adi` instruction.
		 * 
		 * @param r8 `byte` The amount to add to the accumulator.
		 * @param carry `bool` Whether to add the carry bit to the accumulator.
		 */
		void add(const byte r8, const bool carry = false) noexcept;

		/**
		 * @brief Executes the `sub`, `sbb`, `sui`, or `sbi` instruction.
		 * 
		 * @param r8 `byte` The amount to subtract from the accumulator.
		 * @param carry `bool` Whether to subtract the carry bit from the accumulator.
		 */
		void sub(const byte r8, const bool carry = false) noexcept;

		/**
		 * @brief Executes the `cmp` or `cpi` instruction with operand `r8`.
		 * 
		 * @param r8 `byte`
		 */
		void cmp(const byte r8) noexcept;

		/**
		 * @brief Executes the `ana` or `ani` instruction.
		 *
		 * @param r8 `byte` The value to AND the accumulator with.
		 */
		void logicAnd(const byte r8) noexcept;

		/**
		 * @brief Executes the `ora` or `ori` instruction.
		 * 
		 * @param r8 `byte` The value to OR the accumulator with.
		 */
		void logicOr(const byte r8) noexcept;

		/**
		 * @brief Executes the `xra` or `xri` instruction.
		 * 
		 * @param r8 `byte` The value to XOR the accumulator with.
		 */
		void logicXor(const byte r8) noexcept;

		/**
		 * @brief Pushes `r16` to the stack.
		 * Behaves like the `push` instruction.
		 * 
		 * @param r16 `bytePair` The value to push.
		 */
		void push(const bytePair r16) noexcept;

		/**
		 * @brief Pops to `r16` from the stack.
		 * Behaves like the `pop` instruction.
		 * 
		 * @param r16 `bytePair` The register to pop to.
		 */
		void pop(bytePair& r16) noexcept;

		/**
		 * @brief Executes the `rst` instruction with operand `r8`.
		 * 
		 * @param rstNum `const int` Which reset (from 0 to 8) to use.
		 */
		void rst(const int rstNum) noexcept;

		/**
		 * @brief Executes a conditional `jmp`.
		 * Performs a `jmp` to the address pointed to by the program counter
		 * if `condition` is true.
		 * 
		 * @param condition `bool` Whether to jump. Use `true` for an unconditional jump.
		 */
		void jmp(const bool condition) noexcept;

		/**
		 * @brief Executes a conditional `ret`.
		 * Performs a `ret` if `condition` is true.
		 * 
		 * @param condition `bool` Whether to return. Use `true` for an unconditional return.
		 */
		void ret(const bool condition) noexcept;

		/**
		 * @brief Calls a subroutine.
		 * Executes `call` with the address pointed to by the program counter if
		 * `condition` is true.
		 * 
		 * @param condition `bool` Whether to call. Use `true` for an unconditional call.
		 */
		void call(const bool r8) noexcept;

		/**
		 * @brief Executes an instruction.
		 * @note The program counter is not incremented.
		 * @note Operands are taken from memory, beginning from the byte pointed
		 * to by the program counter.
		 *
		 * @param r8 The instruction to execute.
		 */
		void exec(const byte r8) noexcept;
	};

	/**
	 * @return `T` The `k` lowest bits of `n`.
	 */
	template<typename T>
	T lowBitsOf(const T n, const std::size_t k) noexcept
	{
		return n % (1U << k);
	}

	/**
	 * @return `T` The `k` highest bits of `n`.
	 */
	template<typename T>
	T highBitsOf(const T n, const std::size_t numBits) noexcept
	{
		return n / (1U << (8 * sizeof(T) - numBits));
	}

	/**
	 * @return `T` The `k` lowest bits of `n`.
	 */
	template<typename T>
	T bitOf(const T n, const std::size_t pos) noexcept
	{
		return lowBitsOf(n, pos + 1) >> pos;
	}

	/**
	 * @return `T` The two's complement of `n`.
	 */
	template<typename T>
	T twosComp(const T n) noexcept
	{
		return ~n + 1;
	}
}