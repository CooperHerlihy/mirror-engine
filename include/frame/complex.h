#pragma once

#include "frame/assert.h"
#include "frame/matrix.h"
#include "frame/types.h"
#include "frame/vector.h"

namespace Mirror {

template <typename T> struct Complex {
	T r, i;

	[[nodiscard]] constexpr T& operator[](usize index) noexcept {
		debug_assert(index < 2);
		return *((T*)this + index);
	}
	[[nodiscard]] constexpr const T& operator[](usize index) const noexcept {
		debug_assert(index < 2);
		return *((T*)this + index);
	}

	[[nodiscard]] constexpr static Complex from_angle(const T radians) noexcept {
		return {std::cos(radians), std::sin(radians)};
	}

	[[nodiscard]] constexpr Vec2<T> operator*(const Vec2<T>& v) const noexcept {
		Complex res = *this * Complex{v.x, v.y};
		return {res.r, res.i};
	}
	[[nodiscard]] constexpr Mat2<T> operator*(const Mat2<T>& v) const noexcept {
		return {*this * v.x, *this * v.y};
	}

	[[nodiscard]] constexpr Complex operator+(const Complex& other) const noexcept {
		return {r + other.r, i + other.i};
	}
	[[nodiscard]] constexpr Complex operator-(const Complex& other) const noexcept {
		return {r - other.r, i - other.i};
	}
	[[nodiscard]] constexpr Complex operator*(const Complex& other) const noexcept {
		return {r * other.r - i * other.i, r * other.i + i * other.r};
	}
	[[nodiscard]] constexpr Complex operator/(const Complex& other) const noexcept {
		debug_assert(other.abs_squared() != 0);
		return {(r * other.r + i * other.i) / (other.r * other.r + other.i * other.i),
				(r * other.i - i * other.r) / (other.r * other.r + other.i * other.i)};
	}
	[[nodiscard]] constexpr Complex operator-() const noexcept {
		return Complex{-r, -i};
	}

	constexpr Complex& operator+=(const Complex& other) noexcept {
		r += other.r;
		i += other.i;
		return *this;
	}
	constexpr Complex& operator-=(const Complex& other) noexcept {
		r -= other.r;
		i -= other.i;
		return *this;
	}
	constexpr Complex& operator*=(const Complex& other) noexcept {
		r = r * other.r - i * other.i;
		i = r * other.i + i * other.r;
		return *this;
	}
	constexpr Complex& operator/=(const Complex& other) noexcept {
		debug_assert(other.abs_squared() != 0);
		r = (r * other.r + i * other.i) / (other.r * other.r + other.i * other.i);
		i = (r * other.i - i * other.r) / (other.r * other.r + other.i * other.i);
		return *this;
	}

	[[nodiscard]] constexpr Complex operator+(const T scalar) const noexcept {
		return {r + scalar, i};
	}
	[[nodiscard]] constexpr Complex operator-(const T scalar) const noexcept {
		return {r - scalar, i};
	}
	[[nodiscard]] constexpr Complex operator*(const T scalar) const noexcept {
		return {r * scalar, i * scalar};
	}
	[[nodiscard]] constexpr Complex operator/(const T scalar) const noexcept {
		debug_assert(scalar != 0);
		return {r / scalar, i / scalar};
	}

	constexpr Complex& operator+=(const T scalar) noexcept {
		r += scalar;
		return *this;
	}
	constexpr Complex& operator-=(const T scalar) noexcept {
		r -= scalar;
		return *this;
	}
	constexpr Complex& operator*=(const T scalar) noexcept {
		r *= scalar;
		i *= scalar;
		return *this;
	}
	constexpr Complex& operator/=(const T scalar) noexcept {
		debug_assert(scalar != 0);
		r /= scalar;
		i /= scalar;
		return *this;
	}

	[[nodiscard]] constexpr bool operator==(const Complex& other) const noexcept {
		return r == other.r && i == other.i;
	}
	[[nodiscard]] constexpr bool operator!=(const Complex& other) const noexcept {
		return r != other.r || i != other.i;
	}
	[[nodiscard]] constexpr bool operator<(const Complex& other) const noexcept {
		return abs_squared() < other.abs_squared();
	}
	[[nodiscard]] constexpr bool operator<=(const Complex& other) const noexcept {
		return abs_squared() <= other.abs_squared();
	}
	[[nodiscard]] constexpr bool operator>(const Complex& other) const noexcept {
		return abs_squared() > other.abs_squared();
	}
	[[nodiscard]] constexpr bool operator>=(const Complex& other) const noexcept {
		return abs_squared() >= other.abs_squared();
	}

	[[nodiscard]] constexpr T abs_squared() const noexcept {
		return r * r + i * i;
	}
	[[nodiscard]] constexpr T abs() const noexcept {
		return std::sqrt(abs_squared());
	}
	[[nodiscard]] constexpr Complex normalized() const noexcept {
		debug_assert(r != 0 || i != 0);
		return *this / abs();
	}
	constexpr Complex normalize() noexcept {
		*this = normalized();
		return *this;
	}
	[[nodiscard]] constexpr Complex conjugate() const noexcept {
		return {r, -i};
	}
};

using Complexf = Complex<f32>;

} // namespace Mirror
