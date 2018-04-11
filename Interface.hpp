#pragma once
#include <cstdint>
#include <array>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef interface // Fuck windows
#include <assert.h>

template<typename out, class type>
inline out method(size_t index, type* self)
{
	return reinterpret_cast<out>((*reinterpret_cast<void***>(self))[index]);
}

namespace interface
{
	namespace detail
	{
		struct interface_iterator_t
		{
			void*(* create_interface)();
			char* name;
			interface_iterator_t* next;
		};

		inline uintptr_t follow_jmp(uintptr_t ptr)
		{
			return ptr + 5 + *reinterpret_cast<int*>(ptr + 1);
		}

		inline auto get_interface_list(uintptr_t ptr)
		{
			return **reinterpret_cast<interface_iterator_t***>(ptr + 0x6);
		}

		// Fasterified
		inline size_t extract_version(const char* name)
		{
			size_t version = 0;
			
			for (; *name; ++name)
			{
				if (*name <= '9')
					version = version * 10 + (*name - '0');
			}
			
			return version;
		}
	}

	template<class type>
	type* get(HMODULE dll, const char* name)
	{
		assert(dll);

		auto ptr = reinterpret_cast<uintptr_t>(GetProcAddress(dll, "CreateInterface")) + 0x4;
		assert(ptr - 0x4);

		ptr = detail::follow_jmp(ptr);
		type*  interface = nullptr;
		size_t version = 0;

		for (auto iter = detail::get_interface_list(ptr); iter; iter = iter->next)
		{
			if (std::strstr(iter->name, name))
			{
				if (auto current_version = detail::extract_version(iter->name); current_version > version)
				{
					version = current_version;
					interface = reinterpret_cast<type*>(iter->create_interface());
				}
			}
		}

		return interface;
	}

	namespace deprecated
	{
		namespace detail
		{
			// Highly specialized function, do not use in normal situations
			template<size_t len>
			constexpr auto convert_to_array(const char(&name)[len])
			{
				std::array<char, len + 3> buffer{ 0 };
				for (size_t i = 0; i < len; ++i)
					buffer[i] = name[i];
				return buffer;
			}
		}

		// Brute-force the interface
		template<size_t len>
		void* get(HMODULE dll, const char(&name)[len])
		{
			auto create_interface = reinterpret_cast<void*(*)(const char*, int*)>(GetProcAddress(dll, "CreateInterface"));
			assert(create_interface);

			auto buffer = detail::convert_to_array(name);
			buffer[len - 1] = '0';

			// Don't bruteforce for too long
			for (buffer[len] = '0'; buffer[len] < '3'; ++buffer[len])
				for (buffer[len + 1] = '0'; buffer[len + 1] < '9'; ++buffer[len + 1])
					if (auto ret = create_interface(buffer.data(), nullptr); ret)
						return ret;

			return nullptr;
		}
	}
};