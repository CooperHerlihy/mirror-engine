#pragma once

#include "types.h"

namespace Mirror {

template<typename T>
struct Quaternion {
	T r, i, j, k;

	constexpr Quaternion(const T r = 0, const T i = 0, const T j = 0, const T k = 0) noexcept : r(r), i(i), j(j), k(k) {}

	[[nodiscard]] constexpr T& operator[](usize index) noexcept {
		assert(index < 4);
		return *((T*)this + index);
	}
	[[nodiscard]] constexpr const T& operator[](usize index) const noexcept {
		assert(index < 4);
		return *((T*)this + index);
	}

	[[nodiscard]] constexpr static Quaternion fromAxisAngle(const Vec3<T>& axis, const T radians) noexcept {
		const T sinr = std::sin(radians / 2);
		return {
			std::cos(radians / 2),
			axis.x * sinr,
			axis.y * sinr,
			axis.z * sinr,
		};
	}
	[[nodiscard]] constexpr Vec3<T> operator*(const Vec3<T>& v) const noexcept {
		Quaternion vec(0, v.x, v.y, v.z);
		Quaternion result = *this * vec * conjugate();
		return { result.i, result.j, result.k };
	}
	[[nodiscard]] constexpr Mat3<T> operator*(const Mat3<T>& v) const noexcept {
		return { *this * v.x, *this * v.y, *this * v.z, };
	}

	[[nodiscard]] constexpr Quaternion operator+(const Quaternion& other) const noexcept {
		return (r + other.r, i + other.i, j + other.j, k + other.k);
	}
	[[nodiscard]] constexpr Quaternion operator-(const Quaternion& other) const noexcept {
		return (r - other.r, i - other.i, j - other.j, k - other.k);
	}
	[[nodiscard]] constexpr Quaternion operator*(const Quaternion& other) const noexcept { 
		return {
			r * other.r - i * other.i - j * other.j - k * other.k,
			r * other.i + i * other.r + j * other.k - k * other.j,
			r * other.j - i * other.k + j * other.r + k * other.i,
			r * other.k + i * other.j - j * other.i + k * other.r,
		};
	}
	[[nodiscard]] constexpr Quaternion operator-() const noexcept {
		return Quaternion(-r, -i, -j, -k);
	}

	constexpr Quaternion& operator+=(const Quaternion& other) noexcept {
		r += other.r;
		i += other.i;
		j += other.j;
		k += other.k;
		return *this;
	}
	constexpr Quaternion& operator-=(const Quaternion& other) noexcept {
		r -= other.r;
		i -= other.i;
		j -= other.j;
		k -= other.k;
		return *this;
	}
	constexpr Quaternion& operator*=(const Quaternion& other) noexcept {
		r = r * other.r - i * other.i - j * other.j - k * other.k;
		i = r * other.i + i * other.r + j * other.k - k * other.j;
		j = r * other.j - i * other.k + j * other.r + k * other.i;
		k = r * other.k + i * other.j - j * other.i + k * other.r;
		return *this;
	}

	[[nodiscard]] constexpr Quaternion operator+(const T scalar) const noexcept {
		return (r + scalar, i, j, k);
	}
	[[nodiscard]] constexpr Quaternion operator-(const T scalar) const noexcept {
		return (r - scalar, i, j, k);
	}

	[[nodiscard]] constexpr Quaternion operator*(const T scalar) const noexcept {
		return (r * scalar, i * scalar, j * scalar, k * scalar);
	}
	[[nodiscard]] constexpr Quaternion operator/(const T scalar) const noexcept {
		assert(scalar != 0);
		return (r / scalar, i / scalar, j / scalar, k / scalar);
	}

	constexpr Quaternion& operator+=(const T scalar) noexcept {
		r += scalar;
		return *this;
	}
	constexpr Quaternion& operator-=(const T scalar) noexcept {
		r -= scalar;
		return *this;
	}
	constexpr Quaternion& operator*=(const T scalar) noexcept {
		r *= scalar;
		i *= scalar;
		j *= scalar;
		k *= scalar;
		return *this;
	}
	constexpr Quaternion& operator/=(const T scalar) noexcept {
		assert(scalar != 0);
		r /= scalar;
		i /= scalar;
		j /= scalar;
		k /= scalar;
		return *this;
	}

	[[nodiscard]] constexpr bool operator==(const Quaternion& other) const noexcept {
		return r == other.r && i == other.i && j == other.j && k == other.k;
	}
	[[nodiscard]] constexpr bool operator!=(const Quaternion& other) const noexcept {
		return r != other.r || i != other.i || j != other.j || k != other.k;
	}
	[[nodiscard]] constexpr bool operator<(const Quaternion& other) const noexcept {
		return absSquared() < other.absSquared();
	}
	[[nodiscard]] constexpr bool operator<=(const Quaternion& other) const noexcept {
		return absSquared() <= other.absSquared();
	}
	[[nodiscard]] constexpr bool operator>(const Quaternion& other) const noexcept {
		return absSquared() > other.absSquared();
	}
	[[nodiscard]] constexpr bool operator>=(const Quaternion& other) const noexcept {
		return absSquared() >= other.absSquared();
	}

	[[nodiscard]] constexpr T dot(const Quaternion& other) const noexcept {
		return r * other.r + i * other.i + j * other.j + k * other.k;
	}
	[[nodiscard]] constexpr T absSquared() const noexcept {
		return dot(*this);
	}
	[[nodiscard]] constexpr T abs() const noexcept {
		return std::sqrt(absSquared());
	}
	[[nodiscard]] constexpr Quaternion normalized() const noexcept {
		assert(r != 0 && i != 0 && j != 0 && k != 0);
		return *this / abs();
	}
	constexpr Quaternion& normalize() noexcept {
		*this = normalized();
		return *this;
	}
	[[nodiscard]] constexpr Quaternion conjugate() const noexcept {
		return { r, -i, -j, -k };
	}
};

template<typename T>
using Quat = Quaternion<T>;
using Quatf = Quaternion<f32>;

}

