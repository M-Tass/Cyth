#pragma once
#include <unordered_map>
#include <cctype>

template<typename chr>
struct is_char : std::integral_constant<bool, std::is_same_v<chr, char> || std::is_same_v<chr, wchar_t>>
{};

template<typename chr>
constexpr bool is_char_v = is_char<chr>::value;

namespace hash
{
	namespace detail
	{
		constexpr auto offset = 0x811C9DC5u, prime = 0x01000193u;
	}

	using hash_t = uint32_t;
	enum class type : uint8_t
	{
		fnv1,
		fnv1a
	};

	template<typename chr, class = std::enable_if_t<is_char_v<chr>>>
	constexpr hash_t fnv1(const chr* str)
	{
		hash_t hash = detail::offset;
		for (; *str; hash = (hash * detail::prime) ^ *str++);
		return hash;
	}

	template<typename chr, class = std::enable_if_t<is_char_v<chr>>>
	constexpr hash_t fnv1a(const chr* str)
	{
		hash_t hash = detail::offset;
		for (; *str; hash = (hash ^ *str++) * detail::prime);
		return hash;
	}

	namespace literals
	{
		constexpr auto operator"" _fnv1(const char* str, size_t)
		{
			return fnv1(str);
		}

		constexpr auto operator"" _fnv1(const wchar_t* str, size_t)
		{
			return fnv1(str);
		}

		constexpr auto operator"" _fnv1a(const char* str, size_t)
		{
			return fnv1a(str);
		}

		constexpr auto operator"" _fnv1a(const wchar_t* str, size_t)
		{
			return fnv1a(str);
		}
	}
}


namespace detail
{
	struct RecvProp;
	struct DVariant
	{
		union
		{
			float floating;
			long integer;
			char* string;
			void* data;
			float vector[3];
			int64_t int64;
		};

		int type;
	};

	struct CRecvProxyData
	{
		const RecvProp* prop;
		DVariant value;
		int element, object;
	};

	struct RecvTable
	{
		RecvProp* props;
		int       count;
		void*     decoder;
		char*     table;
		bool      initialized;
		bool      listed;
	};

	using recv_var_proxy_t = void(__cdecl*)(const CRecvProxyData*, void*, void*);

	struct RecvProp
	{
		char* name;
		int type;
		int flags;
		int size;
		bool listed;
		const void* data;
		RecvProp* array;
		void* length;
		recv_var_proxy_t proxy;
		void* table_proxy;
		RecvTable* table;
		int offset, stride, count;
		const char* parent;
	};

	struct ClientClass
	{
		void* create, *event;
		const char* name;
		RecvTable*  table;
		ClientClass* next;
		int id;
	};
}


using get_all_classes_t = detail::ClientClass*(__thiscall*)(void*);

namespace globals
{
	inline std::unordered_map<hash::hash_t, std::unordered_map<hash::hash_t, std::ptrdiff_t>> netvars;
}

namespace netvars
{
	inline void __fastcall store(const char* group, detail::RecvTable* table, std::ptrdiff_t offset = 0)
	{
		for (size_t i = 0; i < table->count; ++i)
		{
			auto prop = &table->props[i];
			
			if (!prop || std::isdigit(*prop->name))
				continue;

			auto child = prop->table;

			if (child && child->count)
				store(group, child, prop->offset);

			hash::hash_t hash = hash::fnv1a(group), name = hash::fnv1a(prop->name);

			if (globals::netvars[hash][name] <= 0 && 0 <= prop->type && prop->type <= 4)
				globals::netvars[hash][name] = static_cast<std::ptrdiff_t>(prop->offset) + offset;
		}
	}
}