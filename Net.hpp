#pragma once
#include <cstdint>
#include <stdlib.h>
#include <type_traits>
#include <string_view>
#include "Interface.hpp"

// It's the dword police
using ulong_t = unsigned long;
using long_t = signed long;

// Unoptimized, near verbose copy of bf_write
namespace detail
{
	template<typename var, typename = std::enable_if_t<std::is_arithmetic_v<var>>>
	var dword_swap(var value)
	{
		return _byteswap_ulong(value);
	}

	inline ulong_t little_dword(ulong_t value)
	{
		int test = 1;
		return (reinterpret_cast<char&>(test) == 1) ? value : dword_swap(value);
	}

	inline ulong_t load_little_dword(const ulong_t* base, size_t index)
	{
		return little_dword(base[index]);
	}

	inline void store_little_dword(ulong_t* base, size_t index, ulong_t dword)
	{
		base[index] = little_dword(dword);
	}
}

class Packet
{
public:
	Packet(void* buffer, int bytes) : overflow(false), assert_on_overflow(true), name(nullptr)
	{
		bytes &= ~3;

		this->data = reinterpret_cast<unsigned long*>(buffer);
		this->bytes = bytes;

		this->bits = bytes << 3;

		this->end = 0;
	}

	inline size_t bits_left()
	{
		return this->bits - this->end;
	}

	inline void write_ulong_bit(uint32_t current_data, size_t count)
	{
		if (this->bits_left() < count)
		{
			this->end = this->bits;
			this->overflow = true;
			return;
		}

		int32_t masked_end = this->end & 31;
		int32_t dword = this->end >> 5;
		this->end += count;

		// Mask in a dword.
		ulong_t* out = &this->data[dword];

		// Rotate data into dword alignment
		current_data = (current_data << masked_end) | (current_data >> (32 - masked_end));

		// Calculate bitmasks for first and second word
		uint32_t _ = 1 << (count - 1);
		uint32_t mask1 = (_ * 2 - 1) << masked_end;
		uint32_t mask2 = (_ - 1) >> (31 - masked_end);

		// Only look beyond current word if necessary (avoid access violation)
		int32_t i = mask2 & 1;
		ulong_t dword1 = detail::load_little_dword(out, 0);
		ulong_t dword2 = detail::load_little_dword(out, i);

		// Drop bits into place
		dword1 ^= (mask1 & (current_data ^ dword1));
		dword2 ^= (mask2 & (current_data ^ dword2));

		// Note reversed order of writes so that dword1 wins if mask2 == 0 && i == 0
		detail::store_little_dword(out, i, dword2);
		detail::store_little_dword(out, 0, dword1);
	}

	inline void write_long_bit(int32_t data, size_t count)
	{
		// Force the sign-extension bit to be correct even in the case of overflow.
		int32_t preserve_bits = (0x7FFFFFFF >> (32 - count));
		int32_t sign_extension = (data >> 31) & ~preserve_bits;
		data &= preserve_bits;
		data |= sign_extension;

		this->write_ulong_bit(data, count);
	}

	inline void write_byte(int32_t value)
	{
		this->write_ulong_bit(value, sizeof(uint8_t) << 3);
	}

	inline void write_char(int32_t value)
	{
		this->write_long_bit(value, sizeof(char) << 3);
	}

	inline void write_string(const char* str)
	{
		if (str)
			for (this->write_char(*str); *str; this->write_char(*++str));
		else
			this->write_char(0);
	}

private:
	unsigned long* data;
	int bytes, bits, end;

	bool overflow, assert_on_overflow;
	const char* name;
};

class Channel
{
public:
	inline int send_data(Packet& packet, bool reliable = false)
	{
		using send_data_t = int(__thiscall*)(void*, Packet&, bool);
		return method<send_data_t>(41, this)(this, packet, reliable);
	}

	inline void set_name(std::string_view name)
	{
		char buffer[5024];
		Packet packet(buffer, std::size(buffer));

		packet.write_ulong_bit(5, 6);
		packet.write_byte(1);
		packet.write_string("name");
		packet.write_string(name.data());

		this->send_data(packet);
	}

	inline void write(std::string_view text)
	{
		char buffer[5024];
		Packet packet(buffer, std::size(buffer));

		packet.write_ulong_bit(4, 6);
		packet.write_string(text.data());

		this->send_data(packet);
	}
};