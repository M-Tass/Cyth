#pragma once
#include "Net.hpp"
#include "fmath.hpp"

struct player_info_t
{
	char name[32];
	int  id;
	char guid[33];
	struct
	{
		uint32_t id;
		char name[32];
	} buddy;
	bool bot;
	bool hltv;
	uint32_t files[4];
	uint8_t  downloaded;
};

class Engine
{
public:
	inline size_t get_local_player()
	{
		using get_local_player_t = size_t(__thiscall*)(void*);
		return method<get_local_player_t>(12, this)(this);
	}

	inline Channel* get_net_channel()
	{
		using get_net_channel_t = Channel*(__thiscall*)(void*);
		return method<get_net_channel_t>(72, this)(this);
	}

	inline bool get_player_info(size_t index, player_info_t* info)
	{
		using get_player_info_t = bool(__thiscall*)(void*, size_t, player_info_t*);
		return method<get_player_info_t>(8, this)(this, index, info);
	}

	inline const matrix4x4_t& get_view_matrix()
	{
		using get_view_matrix_t = const matrix4x4_t&(__thiscall*)(void*);
		return method<get_view_matrix_t>(36, this)(this);
	}

	inline size_t get_max_clients()
	{
		using get_max_clients_t = size_t(__thiscall*)(void*);
		return method<get_max_clients_t>(21, this)(this);
	}

	inline void get_screen_size(int& width, int& height)
	{
		using get_screen_size_t = void(__thiscall*)(void*, int&, int&);
		return method<get_screen_size_t>(5, this)(this, width, height);
	}

	inline bool is_ingame()
	{
		using is_ingame_t = bool(__thiscall*)(void*);
		return method<is_ingame_t>(26, this)(this);
	}
};


namespace globals
{
	inline Engine* engine = nullptr;
}