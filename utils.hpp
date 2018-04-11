#pragma once
#include <cstdint>
#include <Windows.h>
#include <assert.h>
#include <algorithm>
#include <array>
#ifdef __clang__
#define FORCE_INLINE __attribute__((always_inline))
#else
#define FORCE_INLINE __forceinline
#endif

/*
	Todo:
		- Convert to C++17
		- Make pattern evaluate at compile-time
*/
namespace signature
{
	namespace detail
	{
		template<typename chr = char>
		constexpr uint8_t hex(const chr n)
		{
			return static_cast<uint8_t>(9 * (n >> 6) + (n & 15));
		}

		template<typename chr = char>
		constexpr uint8_t eval(const chr* str, const size_t i)
		{
			return str[i * 3] == 63 ? 63u : hex(str[i * 3]) * 16 + hex(str[i * 3 + 1]);
		}

		template<typename chr = char, size_t... n>
		constexpr std::array<uint8_t, sizeof...(n)> stream(const chr* str, std::index_sequence<n...>)
		{
			return{ eval(str, n)... };
		}

		template<typename chr = char, size_t len>
		constexpr auto convert(const chr(&str)[len])
		{
			static_assert(!(len % 3), "Pattern length fail");
			return stream(str, std::make_index_sequence<len / 3>());
		}
	}

	template<typename chr = char, size_t len>
	uintptr_t search(const uint8_t* start, const size_t size, std::array<chr, len>& pattern)
	{
		// auto pattern = detail::convert(signature);
		auto result = std::search(start, start + size, pattern.begin(), pattern.end(), [](uint8_t lhs, uint8_t rhs)
		{
			return lhs == rhs || rhs == '?';
		});

		return result == start ? 0 : reinterpret_cast<uintptr_t>(result);
	}

	template<typename chr = char, size_t len>
	uintptr_t search(HMODULE module, std::array<chr, len> pattern)
	{
		assert(module);

		uint8_t* start = reinterpret_cast<uint8_t*>(module);
		auto	 hdr   = reinterpret_cast<PIMAGE_NT_HEADERS>(start + reinterpret_cast<PIMAGE_DOS_HEADER>(start)->e_lfanew);
		size_t   size  = hdr->OptionalHeader.SizeOfImage;

		// auto pattern = detail::convert(signature);
		auto result = std::search(start, start + size, pattern.begin(), pattern.end(), [](uint8_t _, uint8_t __)
		{
			return _ == __ || __ == 63;
		});

		return result == start ? 0 : reinterpret_cast<uintptr_t>(result);
	}
}

/*
	Currently unused
*/
class Hook
{
private:
	size_t count = 0ull;
	uintptr_t* original = nullptr;
	uintptr_t** base = nullptr;
	std::unique_ptr<uintptr_t[]> table = nullptr;

	void __init__()
	{
		for (; (*this->base)[this->count]; ++this->count);

		this->original = *this->base;
		this->table = std::make_unique<uintptr_t[]>(this->count);

		std::copy(this->original, this->original + this->count, this->table.get());
		*this->base = table.get();
	}

public:
	template<typename var, typename = std::enable_if_t<std::is_pointer_v<var> || (std::is_integral_v<var> && sizeof(var) == sizeof(void*))>>
	Hook(var address) noexcept : base(reinterpret_cast<uintptr_t**>(address))
	{
		this->__init__();
	}

	Hook() = default;
	Hook(Hook&&) = delete;
	Hook(Hook&)  = delete;

	~Hook() noexcept
	{
		*this->base = this->original;
	}

	template<typename var = void*>
	const var get(const size_t index) const noexcept
	{
		return index > this->count ? nullptr : reinterpret_cast<var>(this->original[index]);
	}

	bool detour(const size_t index, void* function) noexcept
	{
		if (index > this->count)
			return false;

		this->table[index] = reinterpret_cast<uintptr_t>(function);
		return true;
	}

	bool remove(const size_t index) noexcept
	{
		if (index > this->count)
			return false;

		this->table[index] = this->original[index];
		return true;
	}

	uintptr_t& operator[] (const size_t index)
	{
		assert(index > this->count);

		return this->table[index];
	}

	const uintptr_t& operator[] (const size_t index) const
	{
		assert(index > this->count);

		return this->table[index];
	}

	inline const size_t total() const
	{
		return this->count;
	}

	inline void* operator& () const noexcept
	{
		return this->base;
	}
};