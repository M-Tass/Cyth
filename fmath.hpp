#pragma once
#include <cstdint>
#include <type_traits>
#include <algorithm>
#include <stdexcept>
#include <xmmintrin.h>
#include <optional>

using matrix4x4_t = float[16];
using matrix3x4_t = float[3][4];

constexpr float radians = 3.14159265f / 180.f;

namespace fmath
{
	inline float __fastcall sqrt(float x)
	{
		auto half  = x * 0.5f;
		auto rsqrt = x;
		_mm_store_ss(&rsqrt, _mm_rsqrt_ss(_mm_load_ss(&rsqrt)));
		return x * rsqrt * (1.5f - (half * rsqrt * rsqrt));
	}
}

/*
	Some experiments with std::enable_if / SFINAE. Nothing of importance
*/
class Vector
{
public:
	template<typename... vars, typename = std::enable_if_t<std::conjunction_v<std::is_arithmetic<vars>...>>>
	Vector(vars... args) noexcept : offsets{ static_cast<float>(args)... }
	{}

	Vector(const Vector& other) noexcept
	{
		this->offsets[0] = other[0];
		this->offsets[1] = other[1];
		this->offsets[2] = other[2];
	}

	inline float& operator[] (const size_t index) noexcept
	{
		return this->offsets[index];
	}

	inline const float& operator[] (const size_t index) const noexcept
	{
		return this->offsets[index];
	}

	inline const float& at(const size_t index) const
	{
		if (index > 2)
			throw std::out_of_range("Index out of range");
		return this->offsets[index];
	}

	inline float& at(const size_t index)
	{
		if (index > 2)
			throw std::out_of_range("Index out of range");
		return this->offsets[index];
	}

	inline decltype(auto) operator+= (const Vector& other) noexcept
	{
		this->offsets[0] += other[0];
		this->offsets[1] += other[1];
		this->offsets[2] += other[2];
		return *this;
	}

	inline decltype(auto) operator-= (const Vector& other) noexcept
	{
		this->offsets[0] -= other[0];
		this->offsets[1] -= other[1];
		this->offsets[2] -= other[2];
		return *this;
	}

	inline decltype(auto) operator*= (const Vector& other) noexcept
	{
		this->offsets[0] *= other[0];
		this->offsets[1] *= other[1];
		this->offsets[2] *= other[2];
		return *this;
	}

	inline decltype(auto) operator/= (const Vector& other) noexcept
	{
		this->offsets[0] /= other[0];
		this->offsets[1] /= other[1];
		this->offsets[2] /= other[2];
		return *this;
	}

	template<typename var>
	inline std::enable_if_t<std::is_arithmetic_v<var>, Vector&> operator*= (const var arg) noexcept
	{
		auto x = static_cast<float>(arg);
		this->offsets[0] *= x;
		this->offsets[1] *= x;
		this->offsets[2] *= x;
		return *this;
	}

	template<typename var>
	inline std::enable_if_t<std::is_arithmetic_v<var>, Vector&> operator/= (const var arg) noexcept
	{
		auto x = static_cast<float>(arg);
		this->offsets[0] /= x;
		this->offsets[1] /= x;
		this->offsets[2] /= x;
		return *this;
	}

	inline Vector operator+ (const Vector& other) const noexcept
	{
		return { this->offsets[0] + other[0], this->offsets[1] + other[1], this->offsets[2] + other[2] };
	}

	inline Vector operator- (const Vector& other) const noexcept
	{
		return { this->offsets[0] - other[0], this->offsets[1] - other[1], this->offsets[2] - other[2] };
	}

	inline Vector operator* (const Vector& other) const noexcept
	{
		return { this->offsets[0] * other[0], this->offsets[1] * other[1], this->offsets[2] * other[2] };
	}

	inline Vector operator/ (const Vector& other) const noexcept
	{
		return { this->offsets[0] / other[0], this->offsets[1] / other[1], this->offsets[2] / other[2] };
	}

	template<typename var>
	inline std::enable_if_t<std::is_arithmetic_v<var>, Vector> operator* (const var arg) const noexcept
	{
		auto x = static_cast<float>(arg);
		return { this->offsets[0] * x, this->offsets[1] * x, this->offsets[2] * x };
	}

	template<typename var>
	inline std::enable_if_t<std::is_arithmetic_v<var>, Vector> operator/ (const var arg) const noexcept
	{
		auto x = static_cast<float>(arg);
		return { this->offsets[0] / x, this->offsets[1] / x, this->offsets[2] / x };
	}

	inline float length() const noexcept
	{
		auto sum =
			this->offsets[0] * this->offsets[0] +
			this->offsets[1] * this->offsets[1] +
			this->offsets[2] * this->offsets[2];
		
		return fmath::sqrt(sum);
	}

	inline Vector& normalize() noexcept
	{
		auto len = 
			this->offsets[0] * this->offsets[0] +
			this->offsets[1] * this->offsets[1] +
			this->offsets[2] * this->offsets[2];

		if (len == 0.f)
			return *this;

		auto half = len * 0.5f;
		_mm_store_ss(&len, _mm_rsqrt_ss(_mm_load_ss(&len)));
		len *= 1.5f - (half * len * len);

		*this *= len;
		return *this;
	}

	inline float dot(const Vector& other) const noexcept
	{
		return this->offsets[0] * other[0] + this->offsets[1] * other[1] + this->offsets[2] * other[2];
	}

	inline float dot(const float* other) const noexcept
	{
		return this->offsets[0] * other[0] + this->offsets[1] * other[1] + this->offsets[2] * other[2];
	}

	inline void rotate(float yaw) noexcept
	{
		yaw *= radians;
		float s = std::sin(yaw), c = std::cos(yaw);

		float x = this->offsets[0], y = this->offsets[1];

		this->offsets[0] = (x * c) - (y * s);
		this->offsets[1] = (x * s) + (y * c);
	}

private:
	float offsets[3];
};

class Angle
{
public:
	template<typename... vars, typename = std::enable_if_t<std::conjunction_v<std::is_arithmetic<vars>...>>>
	Angle(vars... args) noexcept : offsets{ static_cast<float>(args)... }
	{}

	Angle(const Angle& other) noexcept
	{
		this->offsets[0] = other[0];
		this->offsets[1] = other[1];
		this->offsets[2] = other[2];
	}

	inline float& operator[] (const size_t index) noexcept
	{
		return this->offsets[index];
	}

	inline const float& operator[] (const size_t index) const noexcept
	{
		return this->offsets[index];
	}

	inline const float& at(const size_t index) const
	{
		if (index > 2)
			throw std::out_of_range("Index out of range");
		return this->offsets[index];
	}

	inline float& at(const size_t index)
	{
		if (index > 2)
			throw std::out_of_range("Index out of range");
		return this->offsets[index];
	}

	inline decltype(auto) operator+= (const Angle& other) noexcept
	{
		this->offsets[0] += other[0];
		this->offsets[1] += other[1];
		this->offsets[2] += other[2];
		return *this;
	}

	inline decltype(auto) operator-= (const Angle& other) noexcept
	{
		this->offsets[0] -= other[0];
		this->offsets[1] -= other[1];
		this->offsets[2] -= other[2];
		return *this;
	}

	inline decltype(auto) operator*= (const Angle& other) noexcept
	{
		this->offsets[0] *= other[0];
		this->offsets[1] *= other[1];
		this->offsets[2] *= other[2];
		return *this;
	}

	inline decltype(auto) operator/= (const Angle& other) noexcept
	{
		this->offsets[0] /= other[0];
		this->offsets[1] /= other[1];
		this->offsets[2] /= other[2];
		return *this;
	}

	template<typename var>
	inline std::enable_if_t<std::is_arithmetic_v<var>, Angle&> operator*= (const var arg) noexcept
	{
		auto x = static_cast<float>(arg);
		this->offsets[0] *= x;
		this->offsets[1] *= x;
		this->offsets[2] *= x;
		return *this;
	}

	template<typename var>
	inline std::enable_if_t<std::is_arithmetic_v<var>, Angle&> operator/= (const var arg) noexcept
	{
		auto x = static_cast<float>(arg);
		this->offsets[0] /= x;
		this->offsets[1] /= x;
		this->offsets[2] /= x;
		return *this;
	}

	inline Angle operator+ (const Angle& other) const noexcept
	{
		return { this->offsets[0] + other[0], this->offsets[1] + other[1], this->offsets[2] + other[2] };
	}

	inline Angle operator- (const Angle& other) const noexcept
	{
		return { this->offsets[0] - other[0], this->offsets[1] - other[1], this->offsets[2] - other[2] };
	}

	inline Angle operator* (const Angle& other) const noexcept
	{
		return { this->offsets[0] * other[0], this->offsets[1] * other[1], this->offsets[2] * other[2] };
	}

	inline Angle operator/ (const Angle& other) const noexcept
	{
		return { this->offsets[0] / other[0], this->offsets[1] / other[1], this->offsets[2] / other[2] };
	}

	template<typename var>
	inline std::enable_if_t<std::is_arithmetic_v<var>, Angle> operator* (const var arg) const noexcept
	{
		auto x = static_cast<float>(arg);
		return { this->offsets[0] * x, this->offsets[1] * x, this->offsets[2] * x };
	}

	template<typename var>
	inline std::enable_if_t<std::is_arithmetic_v<var>, Angle> operator/ (const var arg) const noexcept
	{
		auto x = static_cast<float>(arg);
		return { this->offsets[0] / x, this->offsets[1] / x, this->offsets[2] / x };
	}

	inline decltype(auto) clamp() noexcept
	{
		this->offsets[0] = std::clamp(this->offsets[0], -89.f, 89.f);
		this->offsets[1] = std::remainder(this->offsets[1] + 180.f, 360.f) - 180.f;
		this->offsets[2] = 0.f;
		return *this;
	}

	inline Vector to_vector()
	{
		auto pitch  = this->offsets[0] * radians;
		auto yaw    = this->offsets[1] * radians;
		auto cosine = std::cos(pitch);

		return Vector(cosine * std::cos(yaw), cosine * std::sin(yaw), -std::sin(pitch));
	}

	inline void to_vector(Vector& fwd, Vector& up, Vector& right)
	{
		auto pitch = this->offsets[0] * radians;
		auto yaw   = this->offsets[1] * radians;
		auto roll  = this->offsets[2] * radians;

		auto sp = std::sin(pitch);
		auto cp = std::cos(pitch);

		auto sy = std::sin(yaw);
		auto cy = std::cos(yaw);

		auto sr = std::sin(roll);
		auto cr = std::cos(roll);

		
		fwd[0] = cp * cy;
		fwd[1] = cp * sy;
		fwd[2] = -sp;
		

		right[0] = -sr * sp * cy + cr * sy;
		right[1] = -sr * sp * sy + -cr * cy;
		right[2] = -sr * cp;

		up[0] = cr * sp * cy + sr * sy;
		up[1] = cr * sp * sy + -sr * cy;
		up[2] = cr * cp;
	}

private:
	float offsets[3];
};

namespace fmath
{
	inline void __fastcall vector_transform(const matrix3x4_t& matrix, const Vector& in, Vector& out)
	{
		out[0] = in.dot(matrix[0]) + matrix[0][3];
		out[1] = in.dot(matrix[1]) + matrix[1][3];
		out[2] = in.dot(matrix[2]) + matrix[2][3];
	}
}