#pragma once
#include "d3d9.hpp"
#include "Entities.hpp"
#include "Engine.hpp"
#include "Player.hpp"
#include <algorithm>

/*
	Todo:
		- Optimize code
		- Add a menu
*/
namespace offsets
{
	constexpr std::ptrdiff_t m_rgflCoordinateFrame = 0x308;
}

namespace esp
{
	inline std::optional<ImVec2> __fastcall transform(const matrix4x4_t& matrix, const ImVec2& resolution, const Vector& position)
	{
		float w = matrix[12] * position[0] + matrix[13] * position[1] + matrix[14] * position[2] + matrix[15];

		if (w < 0.01f)
			return {};

		w = 1.f / w;
		float x = (matrix[0] * position[0] + matrix[1] * position[1] + matrix[2] * position[2] + matrix[3]) * w;
		float y = (matrix[4] * position[0] + matrix[5] * position[1] + matrix[6] * position[2] + matrix[7]) * w;

		return ImVec2
		{
			(resolution.x * 0.5f) + (0.5f * x * resolution.x + 0.5f),
			(resolution.y * 0.5f) - (0.5f * y * resolution.y + 0.5f)
		};
	}

	inline bool __fastcall transform(const matrix4x4_t& matrix, const ImVec2& resolution, const Vector& position, ImVec2& out)
	{
		float w = matrix[12] * position[0] + matrix[13] * position[1] + matrix[14] * position[2] + matrix[15];

		if (w < 0.01f)
			return false;

		w = 1.f / w;
		float x = (matrix[0] * position[0] + matrix[1] * position[1] + matrix[2] * position[2] + matrix[3]) * w;
		float y = (matrix[4] * position[0] + matrix[5] * position[1] + matrix[6] * position[2] + matrix[7]) * w;

		out.x = (resolution.x * 0.5f) + (0.5f * x * resolution.x + 0.5f);
		out.y = (resolution.y * 0.5f) - (0.5f * y * resolution.y + 0.5f);

		return true;
	}

	inline uint32_t __fastcall calculate_health_color(int32_t health, int32_t max)
	{
		if (health > max)
			return IM_COL32(32, 115, 255, 255);

		auto multiplier = 510.f / max;

		auto r = (std::min)(static_cast<uint32_t>(multiplier * (max - health)), 255u);
		auto g = (std::min)(static_cast<uint32_t>(multiplier * (health)), 255u);
		return 0xFF000000 + r + (g << 8);
	}

	inline void __fastcall eye_trace(ImDrawList* list, const matrix4x4_t& matrix, const ImVec2& res, const Vector& origin, const Angle& angles)
	{
		constexpr float len = 240.f;
		float pitch = angles[0] * radians;
		float yaw = angles[1] * radians;

		float sp = std::sin(pitch);
		float cp = std::cos(pitch);
		
		float sy = std::sin(yaw);
		float cy = std::cos(yaw);

		Vector dir(cp * cy, cp * sy, -sp);

		if (auto start = transform(matrix, res, origin))
		{
			if (auto end = transform(matrix, res, origin + dir * len))
			{
				list->AddLine(start.value(), end.value(), IM_COL32(255, 255, 255, 255));
			}
		}
	}

	inline void __fastcall draw_bounding_box(const matrix4x4_t& matrix, ImDrawList* list, const ImVec2& res, Player* ply)
	{
		auto& frame   = *reinterpret_cast<matrix3x4_t*>(reinterpret_cast<uintptr_t>(ply) + offsets::m_rgflCoordinateFrame);
		auto& min     = ply->get_oob_min();
		auto& max     = ply->get_oob_max();
		auto health   = ply->get_health();
		auto capacity = ply->get_max_health();
		
		Vector coords[]
		{
			Vector(min),
			Vector(min[0], max[1], min[2]),
			Vector(max[0], max[1], min[2]),
			Vector(max[0], min[1], min[2]),
			Vector(max),
			Vector(min[0], max[1], max[2]),
			Vector(min[0], min[1], max[2]),
			Vector(max[0], min[1], max[2]),
		};

		ImVec2 points[std::size(coords)];

		for (auto& coord : coords)
		{
			fmath::vector_transform(frame, coord, coord);
		}

		for (size_t i = 0; i < std::size(points); ++i)
		{
			if (!transform(matrix, res, coords[i], points[i]))
				return;
		}

		float left = points[0].x, right = points[0].x, bottom = points[0].y, top = points[0].y;

		for (const auto& point : points)
		{
			if (left > point.x)
				left = point.x;
			if (top > point.y)
				top = point.y;
			if (right < point.x)
				right = point.x;
			if (bottom < point.y)
				bottom = point.y;
		}

		list->AddRectFilled({ left - 5.f, bottom }, { left - 2.f, top }, IM_COL32(0, 0, 0, 255));
		auto offset = (std::min)(static_cast<float>(health) / capacity, 1.f);

		if (health > 0)
			list->AddRectFilled(
				{ left - 4.f, bottom - 1.f },
				{ left - 3.f, bottom - offset * (bottom - top - 1.f) },
				calculate_health_color(health, capacity)
			);
		list->AddRect({ left, bottom }, { right, top }, 0xFF000000, 0.f, -1, 3.5f);
		list->AddRect({ left, bottom }, { right, top }, ply->get_team_color().value_or(0xFFFF0000));

		// auto name = std::move(ply->get_name());
		auto name = ply->get_name();
		auto size = ImGui::CalcTextSize(name.c_str());

		list->AddText({ left + .5f * (right - left) - size.x * .5f - 1.f, bottom + size.y * .5f }, 0xFF000000, name.c_str());
		list->AddText({ left + .5f * (right - left) - size.x * .5f + 1.f, bottom + size.y * .5f }, 0xFF000000, name.c_str());
		list->AddText({ left + .5f * (right - left) - size.x * .5f, bottom + size.y * .5f - 1.f }, 0xFF000000, name.c_str());
		list->AddText({ left + .5f * (right - left) - size.x * .5f, bottom + size.y * .5f + 1.f }, 0xFF000000, name.c_str());
		list->AddText({ left + .5f * (right - left) - size.x * .5f, bottom + size.y * .5f }, ply->get_team_color().value_or(0xFFFF0000), name.c_str());

		// auto team = std::move(ply->get_team_name());
		auto team = ply->get_team_name();
		size = ImGui::CalcTextSize(team.c_str());
		
		list->AddText({ left + .5f * (right - left) - size.x * .5f - 1.f, bottom + size.y * 1.25f }, 0xFF000000, team.c_str());
		list->AddText({ left + .5f * (right - left) - size.x * .5f + 1.f, bottom + size.y * 1.25f }, 0xFF000000, team.c_str());
		list->AddText({ left + .5f * (right - left) - size.x * .5f, bottom + size.y * 1.25f + 1.f }, 0xFF000000, team.c_str());
		list->AddText({ left + .5f * (right - left) - size.x * .5f, bottom + size.y * 1.25f - 1.f }, 0xFF000000, team.c_str());
		list->AddText({ left + .5f * (right - left) - size.x * .5f, bottom + size.y * 1.25f }, ply->get_team_color().value_or(0xFFFF0000), team.c_str());
	}

	inline void __fastcall draw_3d_box(const matrix4x4_t& matrix, ImDrawList* list, const ImVec2& res, Player* ply)
	{
		auto& pos = ply->get_pos();
		auto& min = ply->get_oob_min();
		auto& max = ply->get_oob_max();
		auto  yaw = ply->get_angles()[1];
		
		Vector coords[]
		{
			Vector(min),
			Vector(min[0], max[1], min[2]),
			Vector(max[0], max[1], min[2]),
			Vector(max[0], min[1], min[2]),
			Vector(min[0], min[1], max[2]),
			Vector(min[0], max[1], max[2]),
			Vector(max),
			Vector(max[0], min[1], max[2]),
		};

		ImVec2 points[std::size(coords)];

		for (auto& coord : coords)
		{
			coord.rotate(yaw);
			coord += pos;
		}

		for (size_t i = 0; i < std::size(points); ++i)
		{
			if (!transform(matrix, res, coords[i], points[i]))
				return;
		}

		auto col = ply->get_team_color().value_or(0xFFFF0000);

		for (size_t i = 1; i <= 4; ++i)
		{
			list->AddLine(points[i - 1], points[i % 4], col);
			list->AddLine(points[i - 1], points[i + 3], col);
			list->AddLine(points[i + 3], points[i % 4 + 4], col);
		}
	}
	
}

inline void render()
{
	if (!globals::engine->is_ingame())
		return;

	auto&  io = ImGui::GetIO();
	ImVec2 resolution{ io.DisplaySize.x, io.DisplaySize.y };

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::Begin("ESP", reinterpret_cast<bool*>(true), ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);

	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
	ImGui::SetWindowSize(ImVec2(resolution.x, resolution.y), ImGuiSetCond_Always);


	auto list = ImGui::GetWindowDrawList();
	{
		list->AddText({ 100.f, 100.f }, 0xFFFFD2FF, "ENGAGED");
		auto glua = globals::lua->create_interface(lua::type::client);
		// next part fixes incorrect view matrix on servers that were previously buggy (still unknown if a specific addon causes this but seems it's not anticheat related)
		if (glua) {
			glua->PushSpecial(lua::special::glob);
			glua->GetField(-1, "cam");
			glua->GetField(-1, "Start3D");
			glua->Call(0, 1);
			glua->Pop();
		}
		auto& matrix = globals::engine->get_view_matrix();
		if (glua) {
			glua->GetField(-1, "End3D");
			glua->Call(0, 1);
			glua->Pop(3);
		}
		auto local = globals::engine->get_local_player();
		for (size_t i = 1; i <= globals::engine->get_max_clients(); ++i)
		{
			auto player = reinterpret_cast<Player*>(globals::entities->get_entity(i));
			
			if (i == local || !player || player->is_dormant())
				continue;

			/*
			player_info_t info{ 0 };
			if (!globals::engine->get_player_info(i, &info))
				continue;
			*/

			esp::draw_bounding_box(matrix, list, resolution, player);
		}
	}
	list->PushClipRectFullScreen();

	ImGui::End();
	ImGui::PopStyleColor();
}
