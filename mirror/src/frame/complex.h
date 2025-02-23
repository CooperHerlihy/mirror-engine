#pragma once

#include "types.h"

#include "vector.h"

namespace Mirror {

template<typename T>
struct Complex {
	T r, i;

	[[nodiscard]] constexpr T& operator[](usize index) noexcept {
		assert(index < 2);
		return *((T*)this + index);
	}
	[[nodiscard]] constexpr const T& operator[](usize index) const noexcept {
		assert(index < 2);
		return *((T*)this + index);
	}

	[[nodiscard]] constexpr static Complex fromAngle(const T radians) noexcept {
		return {
			std::cos(radians),
			std::sin(radians)
		};
	}
	[[nodiscard]] constexpr Vec2<T> operator*(const Vec2<T>& v) const noexcept {
		Complex vec{ v.x, v.y };
		Complex result = *this * vec;
		return { vec.r, vec.i };
	}
	[[nodiscard]] constexpr Mat2<T> operator*(const Mat2<T>& v) const noexcept {
		return { *this * v.x, *this * v.y };
	}

	[[nodiscard]] constexpr Complex operator+(const Complex& other) const noexcept {
		return (r + other.r, i + other.i);
	}
	[[nodiscard]] constexpr Complex operator-(const Complex& other) const noexcept {
		return (r - other.r, i - other.i);
	}
	[[nodiscard]] constexpr Complex operator*(const Complex& other) const noexcept {
		return (r * other.r - i * other.i, r * other.i + i * other.r);
	}
	[[nodiscard]] constexpr Complex operator/(const Complex& other) const noexcept {
		assert(other.absSquared() != 0);
		return {
			(r * other.r + i * other.i) / (other.r * other.r + other.i * other.i),
			(r * other.i - i * other.r) / (other.r * other.r + other.i * other.i)
		};
	}
	[[nodiscard]] constexpr Complex operator-() const noexcept {
		return Complex(-r, -i);
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
		assert(other.absSquared() != 0);
		r = (r * other.r + i * other.i) / (other.r * other.r + other.i * other.i);
		i = (r * other.i - i * other.r) / (other.r * other.r + other.i * other.i);
		return *this;
	}

	[[nodiscard]] constexpr Complex operator+(const T scalar) const noexcept {
		return (r + scalar, i);
	}
	[[nodiscard]] constexpr Complex operator-(const T scalar) const noexcept {
		return (r - scalar, i);
	}
	[[nodiscard]] constexpr Complex operator*(const T scalar) const noexcept {
		return (r * scalar, i * scalar);
	}
	[[nodiscard]] constexpr Complex operator/(const T scalar) const noexcept {
		assert(scalar != 0);
		return (r / scalar, i / scalar);
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
		assert(scalar != 0);
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
		return absSquared() < other.absSquared();
	}
	[[nodiscard]] constexpr bool operator<=(const Complex& other) const noexcept {
		return absSquared() <= other.absSquared();
	}
	[[nodiscard]] constexpr bool operator>(const Complex& other) const noexcept {
		return absSquared() > other.absSquared();
	}
	[[nodiscard]] constexpr bool operator>=(const Complex& other) const noexcept {
		return absSquared() >= other.absSquared();
	}

	[[nodiscard]] constexpr T absSquared() const noexcept {
		return r * r + i * i;
	}
	[[nodiscard]] constexpr T abs() const noexcept {
		return std::sqrt(absSquared());
	}
	[[nodiscard]] constexpr Complex normalized() const noexcept {
		return (r == 0 && i == 0) ? *this : *this / abs();
	}
	constexpr Complex normalize() noexcept {
		*this = normalized();
		return *this;
	}
	[[nodiscard]] constexpr Complex conjugate() const noexcept {
		return (r, -i);
	}
};

using Complexf = Complex<f32>;

}

