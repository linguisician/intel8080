/*
Copyright (C) 2022 Weiju Wang.

This file is part of `intel8080`.

`intel8080` is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

`intel8080` is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. 
*/

/*
References:
- Intel 8080 (Wikipedia): https://en.wikipedia.org/wiki/Intel_8080
- Intel 8080 OPCODES: https://pastraiser.com/cpu/i8080/i8080_opcodes.html
- Intel 8080 Assembly Language Programming Manual: https://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf
- Intel 8080 Microcomputer Systems User's Manual, September 1975: http://bitsavers.trailing-edge.com/components/intel/MCS80/98-153B_Intel_8080_Microcomputer_Systems_Users_Manual_197509.pdf
...and lots of Stack Overflow.
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

namespace intel8080
{
    enum flag
    {
        sign = 7,
        zero = 6,
        auxCarry = 4,
        parity = 2,
        carry = 0
    };

    union reg
    {
        std::uint8_t pair[2];
        std::uint16_t full;

        std::uint8_t& low(void) noexcept;
        std::uint8_t& high(void) noexcept;
    };

    class invalidAsm: public std::exception
    {
    public:
        const std::size_t lineNum;

        invalidAsm(const std::size_t);

        virtual const char* what(void) const noexcept override;
    };

    class cpu
    {
    public:
        std::uint16_t SP = 0, PC = 0;

        std::uint8_t* ram;
        std::function<void(const std::uint8_t port, const std::uint8_t data)> portOutputHandler;
        std::function<std::uint8_t(const std::uint8_t port)> portInputHandler;

        cpu(typeof portInputHandler, typeof portOutputHandler, typeof ram = nullptr) noexcept;

        std::uint16_t& PSW(void) noexcept;
        std::uint16_t& BC(void) noexcept;
        std::uint16_t& DE(void) noexcept;
        std::uint16_t& HL(void) noexcept;

        std::uint8_t& A(void) noexcept;
        std::uint8_t& flags(void) noexcept;
        std::uint8_t& B(void) noexcept;
        std::uint8_t& C(void) noexcept;
        std::uint8_t& D(void) noexcept;
        std::uint8_t& E(void) noexcept;
        std::uint8_t& H(void) noexcept;
        std::uint8_t& L(void) noexcept;

        std::uint8_t& atHL(void) noexcept;
        std::uint16_t& atSP(void) noexcept;

        int getFlag(const flag f) noexcept;
        void setFlag(const flag, const bool condition) noexcept;

        void load(const std::size_t, const std::initializer_list<std::uint8_t>&) noexcept;
        void load(const std::size_t, const std::string&);
        void interrupt(const std::uint8_t);
        void step(void);

        #if INTEL8080_DEBUG__
            void dump(void) noexcept;
        #endif

    private:

        reg _PSW, _BC, _DE, _HL;
        bool interruptsEnabled = false;
        bool interruptPending = false;
        std::uint8_t interruptVector;
        bool halted;

        std::uint8_t get8(void) noexcept;
        std::uint16_t get16(void) noexcept;

        template<typename T>
        void updateFlags(const T) noexcept;

        template<typename T>
        void updateCarryFlag(const T, const T) noexcept;

        template<typename T>
        void updateAuxCarryFlag(const T, const T) noexcept;

        void inr(std::uint8_t&) noexcept;
        void dcr(std::uint8_t&) noexcept;
        void dad(std::uint16_t&) noexcept;
        void add(const std::uint8_t, const bool = false) noexcept;
        void sub(const std::uint8_t, const bool = false) noexcept;
        void cmp(const std::uint8_t) noexcept;
        void logicAnd(const std::uint8_t) noexcept;
        void logicOr(const std::uint8_t) noexcept;
        void logicXor(const std::uint8_t) noexcept;
        void push(const std::uint16_t) noexcept;
        void pop(std::uint16_t&) noexcept;
        void rst(const std::uint8_t) noexcept;
        void jmp(const bool) noexcept;
        void ret(const bool) noexcept;
        void call(const bool) noexcept;

        void exec(const std::uint8_t) noexcept;
    };

    template<typename T>
    T lowBitsOf(const T n, const std::size_t numBits) noexcept
    {
        return n % (1U << numBits);
    }

    template<typename T>
    T highBitsOf(const T n, const std::size_t numBits) noexcept
    {
        return n / (1U << (8 * sizeof(T) - numBits));
    }

    template<typename T>
    T bitOf(const T n, const std::size_t pos) noexcept
    {
        return lowBitsOf(n, pos + 1) >> pos;
    }

    template<typename T>
    T twosComp(const T n) noexcept
    {
        return ~n + 1;
    }

    std::uint16_t swapBytes(const std::uint16_t);

    std::vector<std::uint8_t> assemble(const std::string&);
}