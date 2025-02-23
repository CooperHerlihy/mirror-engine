#pragma once

#include "types.h"

namespace Mirror {

template<typename T, iptr N>
struct Vector {};

template<typename T>
struct Vector<T, 2> {
	T x, y;

	constexpr Vector<T, 2>() noexcept : x(0), y(0) {}
	constexpr Vector<T, 2>(const T scalar) noexcept : x(scalar), y(scalar) {}
	constexpr Vector<T, 2>(const T x, const T y) noexcept : x(x), y(y) {}

	[[nodiscard]] constexpr T& operator[](iptr index) noexcept {
		assert(index < 2);
		return *((T*)this + index);
	}
	[[nodiscard]] constexpr const T& operator[](iptr index) const noexcept {
		assert(index < 2);
		return *((T*)this + index);
	}

	[[nodiscard]] constexpr Vector<T, 2> operator+(const Vector<T, 2>& other) const noexcept {
		return { x + other.x, y + other.y };
	}
	[[nodiscard]] constexpr Vector<T, 2> operator-(const Vector<T, 2>& other) const noexcept {
		return { x - other.x, y - other.y };
	}
	[[nodiscard]] constexpr Vector<T, 2> operator*(const Vector<T, 2>& other) const noexcept {
		return { x * other.x, y * other.y };
	}
	[[nodiscard]] constexpr Vector<T, 2> operator/(const Vector<T, 2>& other) const noexcept {
		assert(other.x != 0 && other.y != 0);
		return { x / other.x, y / other.y };
	}
	[[nodiscard]] constexpr Vector<T, 2> operator-() const noexcept {
		return Vector<T, 2>(-x, -y);
	}

	constexpr Vector<T, 2>& operator+=(const Vector<T, 2>& other) noexcept {
		x += other.x;
		y += other.y;
		return *this;
	}
	constexpr Vector<T, 2>& operator-=(const Vector<T, 2>& other) noexcept {
		x -= other.x;
		y -= other.y;
		return *this;
	}
	constexpr Vector<T, 2>& operator*=(const Vector<T, 2>& other) noexcept {
		x *= other.x;
		y *= other.y;
		return *this;
	}
	constexpr Vector<T, 2>& operator/=(const Vector<T, 2>& other) noexcept {
		assert(other.x != 0 && other.y != 0);
		x /= other.x;
		y /= other.y;
		return *this;
	}

	[[nodiscard]] constexpr Vector<T, 2> operator+(const T scalar) const noexcept {
		return { x + scalar, y + scalar };
	}
	[[nodiscard]] constexpr Vector<T, 2> operator-(const T scalar) const noexcept {
		return { x - scalar, y - scalar };
	}
	[[nodiscard]] constexpr Vector<T, 2> operator*(const T scalar) const noexcept {
		return { x * scalar, y * scalar };
	}
	[[nodiscard]] constexpr Vector<T, 2> operator/(const T scalar) const noexcept {
		assert(scalar != 0);
		return { x / scalar, y / scalar };
	}

	constexpr Vector<T, 2>& operator+=(const T scalar) noexcept {
		x += scalar;
		y += scalar;
		return *this;
	}
	constexpr Vector<T, 2>& operator-=(const T scalar) noexcept {
		x -= scalar;
		y -= scalar;
		return *this;
	}
	constexpr Vector<T, 2>& operator*=(const T scalar) noexcept {
		x *= scalar;
		y *= scalar;
		return *this;
	}
	constexpr Vector<T, 2>& operator/=(const T scalar) noexcept {
		assert(scalar != 0);
		x /= scalar;
		y /= scalar;
		return *this;
	}

	[[nodiscard]] constexpr bool operator==(const Vector<T, 2>& other) const noexcept {
		return x == other.x && y == other.y;
	}
	[[nodiscard]] constexpr bool operator!=(const Vector<T, 2>& other) const noexcept {
		return x != other.x || y != other.y;
	}
	[[nodiscard]] constexpr bool operator<(const Vector<T, 2>& other) const noexcept {
		return lengthSquared() < other.lengthSquared();
	}
	[[nodiscard]] constexpr bool operator<=(const Vector<T, 2>& other) const noexcept {
		return lengthSquared() <= other.lengthSquared();
	}
	[[nodiscard]] constexpr bool operator>(const Vector<T, 2>& other) const noexcept {
		return lengthSquared() > other.lengthSquared();
	}
	[[nodiscard]] constexpr bool operator>=(const Vector<T, 2>& other) const noexcept {
		return lengthSquared() >= other.lengthSquared();
	}

	[[nodiscard]] constexpr T dot(const Vector<T, 2>& other) const noexcept {
		return x * other.x + y * other.y;
	}
	[[nodiscard]] constexpr T lengthSquared() const noexcept {
		return dot(*this);
	}
	[[nodiscard]] constexpr T length() const noexcept {
		return std::sqrt(lengthSquared());
	}
	[[nodiscard]] constexpr Vector<T, 2> normalized() const noexcept {
		assert(x != 0 || y != 0);
		return *this / length();
	}
	constexpr Vector<T, 2> normalize() noexcept {
		*this = normalized();
		return *this;
	}
};

template<typename T>
using Vec2 = Vector<T, 2>;
using Vec2f = Vector<f32, 2>;

template<typename T>
struct Vector<T, 3> {
	T x, y, z;

	constexpr Vector<T, 3>() noexcept : x(0), y(0), z(0) {}
	constexpr Vector<T, 3>(const T scalar) noexcept : x(scalar), y(scalar), z(scalar) {}
	constexpr Vector<T, 3>(const T x, const T y, const T z) noexcept : x(x), y(y), z(z) {}

	constexpr Vector<T, 3>(const Vec2<T>& vec) noexcept : x(vec.x), y(vec.y), z(0) {}
	constexpr Vector<T, 3>(const Vec2<T>& vec, const T z, const T w) noexcept : x(vec.x), y(vec.y), z(z) {}

	[[nodiscard]] constexpr T& operator[](iptr index) noexcept {
		assert(index < 3);
		return *((T*)this + index);
	}
	[[nodiscard]] constexpr const T& operator[](iptr index) const noexcept {
		assert(index < 3);
		return *((T*)this + index);
	}

	[[nodiscard]] constexpr Vector<T, 3> operator+(const Vector<T, 3>& other) const noexcept {
		return { x + other.x, y + other.y, z + other.z };
	}
	[[nodiscard]] constexpr Vector<T, 3> operator-(const Vector<T, 3>& other) const noexcept {
		return { x - other.x, y - other.y, z - other.z };
	}
	[[nodiscard]] constexpr Vector<T, 3> operator*(const Vector<T, 3>& other) const noexcept {
		return { x * other.x, y * other.y, z * other.z };
	}
	[[nodiscard]] constexpr Vector<T, 3> operator/(const Vector<T, 3>& other) const noexcept {
		assert(other.x != 0 && other.y != 0 && other.z != 0);
		return { x / other.x, y / other.y, z / other.z };
	}
	[[nodiscard]] constexpr Vector<T, 3> operator-() const noexcept { return Vector<T, 3>(-x, -y, -z); }

	constexpr Vector<T, 3>& operator+=(const Vector<T, 3>& other) noexcept {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	constexpr Vector<T, 3>& operator-=(const Vector<T, 3>& other) noexcept {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	constexpr Vector<T, 3>& operator*=(const Vector<T, 3>& other) noexcept {
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}
	constexpr Vector<T, 3>& operator/=(const Vector<T, 3>& other) noexcept {
		assert(other.x != 0 && other.y != 0 && other.z != 0);
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}

	[[nodiscard]] constexpr Vector<T, 3> operator+(const T scalar) const noexcept {
		return { x + scalar, y + scalar, z + scalar };
	}
	[[nodiscard]] constexpr Vector<T, 3> operator-(const T scalar) const noexcept {
		return { x - scalar, y - scalar, z - scalar };
	}
	[[nodiscard]] constexpr Vector<T, 3> operator*(const T scalar) const noexcept {
		return { x * scalar, y * scalar, z * scalar };
	}
	[[nodiscard]] constexpr Vector<T, 3> operator/(const T scalar) const noexcept {
		assert(scalar != 0);
		return { x / scalar, y / scalar, z / scalar };
	}

	constexpr Vector<T, 3>& operator+=(const T scalar) noexcept {
		x += scalar;
		y += scalar;
		z += scalar;
		return *this;
	}
	constexpr Vector<T, 3>& operator-=(const T scalar) noexcept {
		x -= scalar;
		y -= scalar;
		z -= scalar;
		return *this;
	}
	constexpr Vector<T, 3>& operator*=(const T scalar) noexcept {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}
	constexpr Vector<T, 3>& operator/=(const T scalar) noexcept {
		assert(scalar != 0);
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}

	[[nodiscard]] constexpr bool operator==(const Vector<T, 3>& other) const noexcept {
		return x == other.x && y == other.y && z == other.z;
	}
	[[nodiscard]] constexpr bool operator!=(const Vector<T, 3>& other) const noexcept {
		return x != other.x || y != other.y || z != other.z;
	}
	[[nodiscard]] constexpr bool operator<(const Vector<T, 3>& other) const noexcept {
		return lengthSquared() < other.lengthSquared();
	}
	[[nodiscard]] constexpr bool operator<=(const Vector<T, 3>& other) const noexcept {
		return lengthSquared() <= other.lengthSquared();
	}
	[[nodiscard]] constexpr bool operator>(const Vector<T, 3>& other) const noexcept {
		return lengthSquared() > other.lengthSquared();
	}
	[[nodiscard]] constexpr bool operator>=(const Vector<T, 3>& other) const noexcept {
		return lengthSquared() >= other.lengthSquared();
	}

	[[nodiscard]] constexpr T dot(const Vector<T, 3>& other) const noexcept {
		return x * other.x + y * other.y + z * other.z;
	}
	[[nodiscard]] constexpr T lengthSquared() const noexcept {
		return dot(*this);
	}
	[[nodiscard]] constexpr T length() const noexcept {
		return std::sqrt(lengthSquared());
	}
	[[nodiscard]] constexpr Vector<T, 3> normalized() const noexcept {
		assert(x != 0 || y != 0 || z != 0);
		return *this / length();
	}
	constexpr Vector<T, 3>& normalize() noexcept {
		*this = normalized();
		return *this;
	}
};

template<typename T>
using Vec3 = Vector<T, 3>;
using Vec3f = Vector<f32, 3>;

template<typename T>
struct Vector<T, 4> {
	T x, y, z, w;

	constexpr Vector<T, 4>() noexcept : x(0), y(0), z(0), w(0) {}
	constexpr Vector<T, 4>(const T scalar) noexcept : x(scalar), y(scalar), z(scalar), w(scalar) {}
	constexpr Vector<T, 4>(const T x, const T y, const T z, const T w) noexcept : x(x), y(y), z(z), w(w) {}

	constexpr Vector<T, 4>(const Vec2<T>& vec) noexcept : x(vec.x), y(vec.y), z(0), w(0) {}
	constexpr Vector<T, 4>(const Vec3<T>& vec) noexcept : x(vec.x), y(vec.y), z(vec.z), w(0) {}
	constexpr Vector<T, 4>(const Vec2<T>& vec, const T z, const T w) noexcept : x(vec.x), y(vec.y), z(z), w(w) {}
	constexpr Vector<T, 4>(const Vec3<T>& vec, const T w) noexcept : x(vec.x), y(vec.y), z(vec.z), w(w) {}

	[[nodiscard]] constexpr T& operator[](iptr index) noexcept {
		assert(index < 4);
		return *((T*)this + index);
	}
	[[nodiscard]] constexpr const T& operator[](iptr index) const noexcept {
		assert(index < 4);
		return *((T*)this + index);
	}

	[[nodiscard]] constexpr Vector<T, 4> operator+(const Vector<T, 4>& other) const noexcept {
		return { x + other.x, y + other.y, z + other.z, w + other.w };
	}
	[[nodiscard]] constexpr Vector<T, 4> operator-(const Vector<T, 4>& other) const noexcept {
		return { x - other.x, y - other.y, z - other.z, w - other.w };
	}
	[[nodiscard]] constexpr Vector<T, 4> operator*(const Vector<T, 4>& other) const noexcept {
		return { x * other.x, y * other.y, z * other.z, w * other.w };
	}
	[[nodiscard]] constexpr Vector<T, 4> operator/(const Vector<T, 4>& other) const noexcept {
		assert(other.x != 0 && other.y != 0 && other.z != 0 && other.w != 0);
		return { x / other.x, y / other.y, z / other.z, w / other.w };
	}
	[[nodiscard]] constexpr Vector<T, 4> operator-() const noexcept { return Vector<T, 4>(-x, -y, -z, -w); }

	constexpr Vector<T, 4>& operator+=(const Vector<T, 4>& other) noexcept {
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}
	constexpr Vector<T, 4>& operator-=(const Vector<T, 4>& other) noexcept {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}
	constexpr Vector<T, 4>& operator*=(const Vector<T, 4>& other) noexcept {
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
		return *this;
	}
	constexpr Vector<T, 4>& operator/=(const Vector<T, 4>& other) noexcept {
		assert(other.x != 0 && other.y != 0 && other.z != 0 && other.w != 0);
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;
		return *this;
	}

	[[nodiscard]] constexpr Vector<T, 4> operator+(const T scalar) const noexcept {
		return { x + scalar, y + scalar, z + scalar, w + scalar };
	}
	[[nodiscard]] constexpr Vector<T, 4> operator-(const T scalar) const noexcept {
		return { x - scalar, y - scalar, z - scalar, w - scalar };
	}
	[[nodiscard]] constexpr Vector<T, 4> operator*(const T scalar) const noexcept {
		return { x * scalar, y * scalar, z * scalar, w * scalar };
	}
	[[nodiscard]] constexpr Vector<T, 4> operator/(const T scalar) const noexcept {
		assert(scalar != 0);
		return { x / scalar, y / scalar, z / scalar, w / scalar };
	}

	constexpr Vector<T, 4>& operator+=(const T scalar) noexcept {
		x += scalar;
		y += scalar;
		z += scalar;
		w += scalar;
		return *this;
	}
	constexpr Vector<T, 4>& operator-=(const T scalar) noexcept {
		x -= scalar;
		y -= scalar;
		z -= scalar;
		w -= scalar;
		return *this;
	}
	constexpr Vector<T, 4>& operator*=(const T scalar) noexcept {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
		return *this;
	}
	constexpr Vector<T, 4>& operator/=(const T scalar) noexcept {
		assert(scalar != 0);
		x /= scalar;
		y /= scalar;
		z /= scalar;
		w /= scalar;
		return *this;
	}

	[[nodiscard]] constexpr bool operator==(const Vector<T, 4>& other) const noexcept {
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}
	[[nodiscard]] constexpr bool operator!=(const Vector<T, 4>& other) const noexcept {
		return x != other.x || y != other.y || z != other.z || w != other.w;
	}
	[[nodiscard]] constexpr bool operator<(const Vector<T, 4>& other) const noexcept {
		return lengthSquared() < other.lengthSquared();
	}
	[[nodiscard]] constexpr bool operator<=(const Vector<T, 4>& other) const noexcept {
		return lengthSquared() <= other.lengthSquared();
	}
	[[nodiscard]] constexpr bool operator>(const Vector<T, 4>& other) const noexcept {
		return lengthSquared() > other.lengthSquared();
	}
	[[nodiscard]] constexpr bool operator>=(const Vector<T, 4>& other) const noexcept {
		return lengthSquared() >= other.lengthSquared();
	}

	[[nodiscard]] constexpr T dot(const Vector<T, 4>& other) const noexcept {
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}
	[[nodiscard]] constexpr T lengthSquared() const noexcept {
		return dot(*this);
	}
	[[nodiscard]] constexpr T length() const noexcept {
		return std::sqrt(lengthSquared());
	}
	[[nodiscard]] constexpr Vector<T, 4> normalized() const noexcept {
		assert(x != 0 || y != 0 || z != 0 || w != 0);
		return *this / length();
	}
	constexpr Vector<T, 4>& normalize() noexcept {
		*this = normalized();
		return *this;
	}
};

template<typename T>
using Vec4 = Vector<T, 4>;
using Vec4f = Vector<f32, 4>;

}

