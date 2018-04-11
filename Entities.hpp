#pragma once
#include "Interface.hpp"

/*
	Todo: change void* to ply_t
*/

class Entities
{
public:
	inline void* get_entity(int idx)
	{
		using get_entity_t = void*(__thiscall*)(void*, int);
		return method<get_entity_t>(3, this)(this, idx);
	}

	inline void* get_entity_from_handle(int idx)
	{
		using get_entity_from_handle_t = void*(__thiscall*)(void*, int);
		return method<get_entity_from_handle_t>(4, this)(this, idx);
	}

	inline size_t get_highest_entity_index()
	{
		using get_highest_entity_index_t = size_t(__thiscall*)(void*);
		return method<get_highest_entity_index_t>(6, this)(this);
	}
};

namespace globals
{
	inline Entities* entities = nullptr;
}