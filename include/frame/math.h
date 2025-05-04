#pragma once

#include "frame/utils.h"

namespace Mirror {

template <typename T, iptr N> struct Vector {};

template <typename T> struct Vector<T, 2> {
	T x, y;

	constexpr Vector<T, 2>() noexcept : x(0), y(0) {}
	constexpr Vector<T, 2>(const T scalar) noexcept : x(scalar), y(scalar) {}
	constexpr Vector<T, 2>(const T x, const T y) noexcept : x(x), y(y) {}

	[[nodiscard]] constexpr T& operator[](iptr index) noexcept {
		debug_assert(index < 2);
		return *((T*)this + index);
	}
	[[nodiscard]] constexpr const T& operator[](iptr index) const noexcept {
		debug_assert(index < 2);
		return *((T*)this + index);
	}

	[[nodiscard]] constexpr Vector<T, 2> operator+(const Vector<T, 2>& other) const noexcept {
		return {x + other.x, y + other.y};
	}
	[[nodiscard]] constexpr Vector<T, 2> operator-(const Vector<T, 2>& other) const noexcept {
		return {x - other.x, y - other.y};
	}
	[[nodiscard]] constexpr Vector<T, 2> operator*(const Vector<T, 2>& other) const noexcept {
		return {x * other.x, y * other.y};
	}
	[[nodiscard]] constexpr Vector<T, 2> operator/(const Vector<T, 2>& other) const noexcept {
		debug_assert(other.x != 0 && other.y != 0);
		return {x / other.x, y / other.y};
	}
	[[nodiscard]] constexpr Vector<T, 2> operator-() const noexcept {
		return {-x, -y};
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
		debug_assert(other.x != 0 && other.y != 0);
		x /= other.x;
		y /= other.y;
		return *this;
	}

	[[nodiscard]] constexpr Vector<T, 2> operator+(const T scalar) const noexcept {
		return {x + scalar, y + scalar};
	}
	[[nodiscard]] constexpr Vector<T, 2> operator-(const T scalar) const noexcept {
		return {x - scalar, y - scalar};
	}
	[[nodiscard]] constexpr Vector<T, 2> operator*(const T scalar) const noexcept {
		return {x * scalar, y * scalar};
	}
	[[nodiscard]] constexpr Vector<T, 2> operator/(const T scalar) const noexcept {
		debug_assert(scalar != 0);
		return {x / scalar, y / scalar};
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
		debug_assert(scalar != 0);
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
		return len_squared() < other.len_squared();
	}
	[[nodiscard]] constexpr bool operator<=(const Vector<T, 2>& other) const noexcept {
		return len_squared() <= other.len_squared();
	}
	[[nodiscard]] constexpr bool operator>(const Vector<T, 2>& other) const noexcept {
		return len_squared() > other.len_squared();
	}
	[[nodiscard]] constexpr bool operator>=(const Vector<T, 2>& other) const noexcept {
		return len_squared() >= other.len_squared();
	}

	[[nodiscard]] constexpr T dot(const Vector<T, 2>& other) const noexcept {
		return x * other.x + y * other.y;
	}
	[[nodiscard]] constexpr T len_squared() const noexcept {
		return dot(*this);
	}
	[[nodiscard]] constexpr T len() const noexcept {
		return std::sqrt(len_squared());
	}
	[[nodiscard]] constexpr Vector<T, 2> normalized() const noexcept {
		debug_assert(x != 0 || y != 0);
		return *this / len();
	}
	constexpr Vector<T, 2> normalize() noexcept {
		*this = normalized();
		return *this;
	}
};

template <typename T> using Vec2 = Vector<T, 2>;
using Vec2f = Vector<f32, 2>;

template <typename T> struct Vector<T, 3> {
	T x, y, z;

	constexpr Vector<T, 3>() noexcept : x(0), y(0), z(0) {}
	constexpr Vector<T, 3>(const T scalar) noexcept : x(scalar), y(scalar), z(scalar) {}
	constexpr Vector<T, 3>(const T x, const T y, const T z) noexcept : x(x), y(y), z(z) {}

	constexpr Vector<T, 3>(const Vec2<T>& vec) noexcept : x(vec.x), y(vec.y), z(0) {}
	constexpr Vector<T, 3>(const Vec2<T>& vec, const T z, const T w) noexcept : x(vec.x), y(vec.y), z(z) {}

	[[nodiscard]] constexpr T& operator[](iptr index) noexcept {
		debug_assert(index < 3);
		return *((T*)this + index);
	}
	[[nodiscard]] constexpr const T& operator[](iptr index) const noexcept {
		debug_assert(index < 3);
		return *((T*)this + index);
	}

	[[nodiscard]] constexpr Vector<T, 3> operator+(const Vector<T, 3>& other) const noexcept {
		return {x + other.x, y + other.y, z + other.z};
	}
	[[nodiscard]] constexpr Vector<T, 3> operator-(const Vector<T, 3>& other) const noexcept {
		return {x - other.x, y - other.y, z - other.z};
	}
	[[nodiscard]] constexpr Vector<T, 3> operator*(const Vector<T, 3>& other) const noexcept {
		return {x * other.x, y * other.y, z * other.z};
	}
	[[nodiscard]] constexpr Vector<T, 3> operator/(const Vector<T, 3>& other) const noexcept {
		debug_assert(other.x != 0 && other.y != 0 && other.z != 0);
		return {x / other.x, y / other.y, z / other.z};
	}
	[[nodiscard]] constexpr Vector<T, 3> operator-() const noexcept {
		return {-x, -y, -z};
	}

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
		debug_assert(other.x != 0 && other.y != 0 && other.z != 0);
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}

	[[nodiscard]] constexpr Vector<T, 3> operator+(const T scalar) const noexcept {
		return {x + scalar, y + scalar, z + scalar};
	}
	[[nodiscard]] constexpr Vector<T, 3> operator-(const T scalar) const noexcept {
		return {x - scalar, y - scalar, z - scalar};
	}
	[[nodiscard]] constexpr Vector<T, 3> operator*(const T scalar) const noexcept {
		return {x * scalar, y * scalar, z * scalar};
	}
	[[nodiscard]] constexpr Vector<T, 3> operator/(const T scalar) const noexcept {
		debug_assert(scalar != 0);
		return {x / scalar, y / scalar, z / scalar};
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
		debug_assert(scalar != 0);
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
		return len_squared() < other.len_squared();
	}
	[[nodiscard]] constexpr bool operator<=(const Vector<T, 3>& other) const noexcept {
		return len_squared() <= other.len_squared();
	}
	[[nodiscard]] constexpr bool operator>(const Vector<T, 3>& other) const noexcept {
		return len_squared() > other.len_squared();
	}
	[[nodiscard]] constexpr bool operator>=(const Vector<T, 3>& other) const noexcept {
		return len_squared() >= other.len_squared();
	}

	[[nodiscard]] constexpr T dot(const Vector<T, 3>& other) const noexcept {
		return x * other.x + y * other.y + z * other.z;
	}
	[[nodiscard]] constexpr T len_squared() const noexcept {
		return dot(*this);
	}
	[[nodiscard]] constexpr T len() const noexcept {
		return std::sqrt(len_squared());
	}
	[[nodiscard]] constexpr Vector<T, 3> normalized() const noexcept {
		debug_assert(x != 0 || y != 0 || z != 0);
		return *this / len();
	}
	constexpr Vector<T, 3>& normalize() noexcept {
		*this = normalized();
		return *this;
	}
};

template <typename T> using Vec3 = Vector<T, 3>;
using Vec3f = Vector<f32, 3>;

template <typename T> struct Vector<T, 4> {
	T x, y, z, w;

	constexpr Vector<T, 4>() noexcept : x(0), y(0), z(0), w(0) {}
	constexpr Vector<T, 4>(const T scalar) noexcept : x(scalar), y(scalar), z(scalar), w(scalar) {}
	constexpr Vector<T, 4>(const T x, const T y, const T z, const T w) noexcept : x(x), y(y), z(z), w(w) {}

	constexpr Vector<T, 4>(const Vec2<T>& vec) noexcept : x(vec.x), y(vec.y), z(0), w(0) {}
	constexpr Vector<T, 4>(const Vec3<T>& vec) noexcept : x(vec.x), y(vec.y), z(vec.z), w(0) {}
	constexpr Vector<T, 4>(const Vec2<T>& vec, const T z, const T w) noexcept : x(vec.x), y(vec.y), z(z), w(w) {}
	constexpr Vector<T, 4>(const Vec3<T>& vec, const T w) noexcept : x(vec.x), y(vec.y), z(vec.z), w(w) {}

	[[nodiscard]] constexpr T& operator[](iptr index) noexcept {
		debug_assert(index < 4);
		return *((T*)this + index);
	}
	[[nodiscard]] constexpr const T& operator[](iptr index) const noexcept {
		debug_assert(index < 4);
		return *((T*)this + index);
	}

	[[nodiscard]] constexpr Vector<T, 4> operator+(const Vector<T, 4>& other) const noexcept {
		return {x + other.x, y + other.y, z + other.z, w + other.w};
	}
	[[nodiscard]] constexpr Vector<T, 4> operator-(const Vector<T, 4>& other) const noexcept {
		return {x - other.x, y - other.y, z - other.z, w - other.w};
	}
	[[nodiscard]] constexpr Vector<T, 4> operator*(const Vector<T, 4>& other) const noexcept {
		return {x * other.x, y * other.y, z * other.z, w * other.w};
	}
	[[nodiscard]] constexpr Vector<T, 4> operator/(const Vector<T, 4>& other) const noexcept {
		debug_assert(other.x != 0 && other.y != 0 && other.z != 0 && other.w != 0);
		return {x / other.x, y / other.y, z / other.z, w / other.w};
	}
	[[nodiscard]] constexpr Vector<T, 4> operator-() const noexcept {
		return Vector<T, 4>(-x, -y, -z, -w);
	}

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
		debug_assert(other.x != 0 && other.y != 0 && other.z != 0 && other.w != 0);
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;
		return *this;
	}

	[[nodiscard]] constexpr Vector<T, 4> operator+(const T scalar) const noexcept {
		return {x + scalar, y + scalar, z + scalar, w + scalar};
	}
	[[nodiscard]] constexpr Vector<T, 4> operator-(const T scalar) const noexcept {
		return {x - scalar, y - scalar, z - scalar, w - scalar};
	}
	[[nodiscard]] constexpr Vector<T, 4> operator*(const T scalar) const noexcept {
		return {x * scalar, y * scalar, z * scalar, w * scalar};
	}
	[[nodiscard]] constexpr Vector<T, 4> operator/(const T scalar) const noexcept {
		debug_assert(scalar != 0);
		return {x / scalar, y / scalar, z / scalar, w / scalar};
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
		debug_assert(scalar != 0);
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
		return len_squared() < other.len_squared();
	}
	[[nodiscard]] constexpr bool operator<=(const Vector<T, 4>& other) const noexcept {
		return len_squared() <= other.len_squared();
	}
	[[nodiscard]] constexpr bool operator>(const Vector<T, 4>& other) const noexcept {
		return len_squared() > other.len_squared();
	}
	[[nodiscard]] constexpr bool operator>=(const Vector<T, 4>& other) const noexcept {
		return len_squared() >= other.len_squared();
	}

	[[nodiscard]] constexpr T dot(const Vector<T, 4>& other) const noexcept {
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}
	[[nodiscard]] constexpr T len_squared() const noexcept {
		return dot(*this);
	}
	[[nodiscard]] constexpr T len() const noexcept {
		return std::sqrt(len_squared());
	}
	[[nodiscard]] constexpr Vector<T, 4> normalized() const noexcept {
		debug_assert(x != 0 || y != 0 || z != 0 || w != 0);
		return *this / len();
	}
	constexpr Vector<T, 4>& normalize() noexcept {
		*this = normalized();
		return *this;
	}
};

template <typename T> using Vec4 = Vector<T, 4>;
using Vec4f = Vector<f32, 4>;

template <typename T, usize Rows, usize Cols> struct Matrix {};

template <typename T> struct Matrix<T, 2, 2> {
	Vec2<T> x, y;

	constexpr Matrix() noexcept : x(0), y(0) {}
	constexpr Matrix(const T scalar) noexcept : x(scalar, 0), y(0, scalar) {}
	constexpr Matrix(const Vec2<T>& x, const Vec2<T>& y) noexcept : x(x), y(y) {}

	[[nodiscard]] constexpr Vec2<T>& operator[](usize col) noexcept {
		debug_assert(col < 2);
		return *((Vec2<T>*)this + col);
	}
	[[nodiscard]] constexpr const Vec2<T>& operator[](usize col) const noexcept {
		debug_assert(col < 2);
		return *((Vec2<T>*)this + col);
	}
	[[nodiscard]] constexpr T& operator()(usize col, usize row) noexcept {
		debug_assert(col < 2 && row < 2);
		return *((T*)((Vec2<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr const T& operator()(usize col, usize row) const noexcept {
		debug_assert(col < 2 && row < 2);
		return *((T*)((Vec2<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr T& at(usize col, usize row) noexcept {
		return operator()(col, row);
	}
	[[nodiscard]] constexpr const T& at(usize col, usize row) const noexcept {
		return operator()(col, row);
	}

	[[nodiscard]] constexpr Vec2<T> operator+(const Matrix<T, 2, 2>& other) const noexcept {
		return {x + other.x, y + other.y};
	}
	[[nodiscard]] constexpr Vec2<T> operator-(const Matrix<T, 2, 2>& other) const noexcept {
		return {x - other.x, y - other.y};
	}
	constexpr Vec2<T>& operator+=(const Matrix<T, 2, 2>& other) noexcept {
		return {x += other.x, y += other.y};
	}
	constexpr Vec2<T>& operator-=(const Matrix<T, 2, 2>& other) noexcept {
		return {x -= other.x, y -= other.y};
	}

	[[nodiscard]] constexpr Vec2<T> operator*(const T scalar) const noexcept {
		return {x * scalar, y * scalar};
	}
	[[nodiscard]] constexpr Vec2<T> operator/(const T scalar) const noexcept {
		debug_assert(scalar != 0);
		return {x / scalar, y / scalar};
	}
	[[nodiscard]] constexpr Vec2<T>& operator*=(const T scalar) noexcept {
		return {x *= scalar, y *= scalar};
	}
	[[nodiscard]] constexpr Vec2<T>& operator/=(const T scalar) noexcept {
		debug_assert(scalar != 0);
		return {x /= scalar, y /= scalar};
	}

	[[nodiscard]] constexpr Matrix<T, 2, 2> operator*(const Matrix<T, 2, 2>& other) const noexcept {
		return {
			{at(0, 0) * other(0, 0) + at(1, 0) * other(0, 1), at(0, 1) * other(0, 0) + at(1, 1) * other(0, 1)},
			{at(0, 0) * other(1, 0) + at(1, 0) * other(1, 1), at(0, 1) * other(1, 0) + at(1, 1) * other(1, 1)},
		};
	}

	[[nodiscard]] constexpr Vec2<T> operator*(const Vec2<T>& v) const noexcept {
		return {
			at(0, 0) * v[0] + at(1, 0) * v[1],
			at(0, 1) * v[0] + at(1, 1) * v[1],
		};
	}

	[[nodiscard]] constexpr Matrix<T, 2, 2> hadamard(const Matrix<T, 2, 2>& other) const noexcept {
		return {x * other.x, y * other.y};
	}

	[[nodiscard]] constexpr T determinant() const noexcept {
		return {at(0, 0) * at(1, 1) - at(1, 0) * at(0, 1)};
	}
	[[nodiscard]] constexpr Vec2<T> row(usize index) const noexcept {
		debug_assert(index < 2);
		return {x[index], y[index]};
	}
	[[nodiscard]] constexpr Matrix<T, 2, 2> transposed() const noexcept {
		return {row(0), row(1)};
	}
	constexpr Matrix<T, 2, 2>& transpose() noexcept {
		*this = transposed();
		return *this;
	}
};

template <typename T> using Mat2 = Matrix<T, 2, 2>;
using Mat2f = Matrix<f32, 2, 2>;

template <typename T> struct Matrix<T, 3, 3> {
	Vec3<T> x, y, z;

	constexpr Matrix() noexcept : x(0), y(0), z(0) {}
	constexpr Matrix(const T scalar) noexcept : x(scalar, 0, 0), y(0, scalar, 0), z(0, 0, scalar) {}
	constexpr Matrix(const Vec3<T>& x, const Vec3<T>& y, const Vec3<T>& z) noexcept : x(x), y(y), z(z) {}

	constexpr Matrix(const Mat2<T>& other) noexcept : x(other.x), y(other.y), z(0, 0, 1) {}

	[[nodiscard]] constexpr Vec3<T>& operator[](usize col) noexcept {
		debug_assert(col < 3);
		return *((Vec3<T>*)this + col);
	}
	[[nodiscard]] constexpr const Vec3<T>& operator[](usize col) const noexcept {
		debug_assert(col < 3);
		return *((Vec3<T>*)this + col);
	}
	[[nodiscard]] constexpr T& operator()(usize col, usize row) noexcept {
		debug_assert(col < 3 && row < 3);
		return *((T*)((Vec3<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr const T& operator()(usize col, usize row) const noexcept {
		debug_assert(col < 3 && row < 3);
		return *((T*)((Vec3<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr T& at(usize col, usize row) noexcept {
		debug_assert(col < 3 && row < 3);
		return operator()(col, row);
	}
	[[nodiscard]] constexpr const T& at(usize col, usize row) const noexcept {
		debug_assert(col < 3 && row < 3);
		return operator()(col, row);
	}

	[[nodiscard]] constexpr Vec3<T> operator+(const Matrix<T, 3, 3>& other) const noexcept {
		return {x + other.x, y + other.y, z + other.z};
	}
	[[nodiscard]] constexpr Vec3<T> operator-(const Matrix<T, 3, 3>& other) const noexcept {
		return {x - other.x, y - other.y, z - other.z};
	}
	constexpr Vec3<T>& operator+=(const Matrix<T, 3, 3>& other) noexcept {
		return {x += other.x, y += other.y, z += other.z};
	}
	constexpr Vec3<T>& operator-=(const Matrix<T, 3, 3>& other) noexcept {
		return {x -= other.x, y -= other.y, z -= other.z};
	}

	[[nodiscard]] constexpr Vec3<T> operator*(const T scalar) const noexcept {
		return {x * scalar, y * scalar, z * scalar};
	}
	[[nodiscard]] constexpr Vec3<T> operator/(const T scalar) const noexcept {
		debug_assert(scalar != 0);
		return {x / scalar, y / scalar, z / scalar};
	}
	[[nodiscard]] constexpr Vec3<T>& operator*=(const T scalar) noexcept {
		return {x *= scalar, y *= scalar, z *= scalar};
	}
	[[nodiscard]] constexpr Vec3<T>& operator/=(const T scalar) noexcept {
		debug_assert(scalar != 0);
		return {x /= scalar, y /= scalar, z /= scalar};
	}

	[[nodiscard]] constexpr Matrix<T, 3, 3> operator*(const Matrix<T, 3, 3>& other) const noexcept {
		return {
			{
				at(0, 0) * other(0, 0) + at(1, 0) * other(0, 1) + at(2, 0) * other(0, 2),
				at(0, 1) * other(0, 0) + at(1, 1) * other(0, 1) + at(2, 1) * other(0, 2),
				at(0, 2) * other(0, 0) + at(1, 2) * other(0, 1) + at(2, 2) * other(0, 2),
			},
			{
				at(0, 0) * other(1, 0) + at(1, 0) * other(1, 1) + at(2, 0) * other(1, 2),
				at(0, 1) * other(1, 0) + at(1, 1) * other(1, 1) + at(2, 1) * other(1, 2),
				at(0, 2) * other(1, 0) + at(1, 2) * other(1, 1) + at(2, 2) * other(1, 2),
			},
			{
				at(0, 0) * other(2, 0) + at(1, 0) * other(2, 1) + at(2, 0) * other(2, 2),
				at(0, 1) * other(2, 0) + at(1, 1) * other(2, 1) + at(2, 1) * other(2, 2),
				at(0, 2) * other(2, 0) + at(1, 2) * other(2, 1) + at(2, 2) * other(2, 2),
			},
		};
	}

	[[nodiscard]] constexpr Vec3<T> operator*(const Vec3<T>& v) const noexcept {
		return {
			at(0, 0) * v[0] + at(1, 0) * v[1] + at(2, 0) * v[2],
			at(0, 1) * v[0] + at(1, 1) * v[1] + at(2, 1) * v[2],
			at(0, 2) * v[0] + at(1, 2) * v[1] + at(2, 2) * v[2],
		};
	}

	[[nodiscard]] constexpr Matrix<T, 3, 3> hadamard(const Matrix<T, 3, 3>& other) const noexcept {
		return {x * other.x, y * other.y, z * other.z};
	}

	[[nodiscard]] constexpr T determinant() const noexcept {
		return {at(0, 0) * at(1, 1) * at(2, 2) - at(2, 0) * at(1, 1) * at(0, 2) + at(1, 0) * at(2, 1) * at(0, 2) -
				at(1, 0) * at(0, 1) * at(2, 2) + at(2, 0) * at(0, 1) * at(1, 2) - at(0, 0) * at(2, 1) * at(1, 2)};
	}
	[[nodiscard]] constexpr Vec3<T> row(usize index) const noexcept {
		debug_assert(index < 3);
		return {x[index], y[index], z[index]};
	}
	[[nodiscard]] constexpr Matrix<T, 3, 3> transposed() const noexcept {
		return {row(0), row(1), row(2)};
	}
	constexpr Matrix<T, 3, 3>& transpose() noexcept {
		*this = transposed();
		return *this;
	}
};

template <typename T> using Mat3 = Matrix<T, 3, 3>;
using Mat3f = Matrix<f32, 3, 3>;

template <typename T> struct Matrix<T, 4, 4> {
	Vec4<T> x, y, z, w;

	constexpr Matrix() noexcept : x(0), y(0), z(0), w(0) {}
	constexpr Matrix(const T scalar) noexcept
		: x(scalar, 0, 0, 0), y(0, scalar, 0, 0), z(0, 0, scalar, 0), w(0, 0, 0, scalar) {}
	constexpr Matrix(const Vec4<T>& x, const Vec4<T>& y, const Vec4<T>& z, const Vec4<T>& w) noexcept
		: x(x), y(y), z(z), w(w) {}

	constexpr Matrix(const Mat2<T>& other) noexcept : x(other.x), y(other.y), z(0, 0, 1, 0), w(0, 0, 0, 1) {}
	constexpr Matrix(const Mat3<T>& other) noexcept : x(other.x), y(other.y), z(other.z), w(0, 0, 0, 1) {}

	[[nodiscard]] constexpr Vec4<T>& operator[](usize col) noexcept {
		debug_assert(col < 4);
		return *((Vec4<T>*)this + col);
	}
	[[nodiscard]] constexpr const Vec4<T>& operator[](usize col) const noexcept {
		debug_assert(col < 4);
		return *((Vec4<T>*)this + col);
	}
	[[nodiscard]] constexpr T& operator()(usize col, usize row) noexcept {
		debug_assert(col < 4 && row < 4);
		return *((T*)((Vec4<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr const T& operator()(usize col, usize row) const noexcept {
		debug_assert(col < 4 && row < 4);
		return *((T*)((Vec4<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr T& at(usize col, usize row) noexcept {
		debug_assert(col < 4 && row < 4);
		return operator()(col, row);
	}
	[[nodiscard]] constexpr const T& at(usize col, usize row) const noexcept {
		debug_assert(col < 4 && row < 4);
		return operator()(col, row);
	}

	[[nodiscard]] constexpr Vec4<T> operator+(const Matrix<T, 4, 4>& other) const noexcept {
		return {x + other.x, y + other.y, z + other.z, w + other.w};
	}
	[[nodiscard]] constexpr Vec4<T> operator-(const Matrix<T, 4, 4>& other) const noexcept {
		return {x - other.x, y - other.y, z - other.z, w - other.w};
	}
	constexpr Vec4<T>& operator+=(const Matrix<T, 4, 4>& other) noexcept {
		return {x += other.x, y += other.y, z += other.z, w += other.w};
	}
	constexpr Vec4<T>& operator-=(const Matrix<T, 4, 4>& other) noexcept {
		return {x -= other.x, y -= other.y, z -= other.z, w -= other.w};
	}

	[[nodiscard]] constexpr Vec4<T> operator*(const T scalar) const noexcept {
		return {x * scalar, y * scalar, z * scalar, w * scalar};
	}
	[[nodiscard]] constexpr Vec4<T> operator/(const T scalar) const noexcept {
		debug_assert(scalar != 0);
		return {x / scalar, y / scalar, z / scalar, w / scalar};
	}
	constexpr Vec4<T>& operator*=(const T scalar) noexcept {
		return {x *= scalar, y *= scalar, z *= scalar, w *= scalar};
	}
	constexpr Vec4<T>& operator/=(const T scalar) noexcept {
		debug_assert(scalar != 0);
		return {x /= scalar, y /= scalar, z /= scalar, w /= scalar};
	}

	[[nodiscard]] constexpr Matrix<T, 4, 4> operator*(const Matrix<T, 4, 4>& other) const noexcept {
		return {
			{
				at(0, 0) * other(0, 0) + at(1, 0) * other(0, 1) + at(2, 0) * other(0, 2) + at(3, 0) * other(0, 3),
				at(0, 1) * other(0, 0) + at(1, 1) * other(0, 1) + at(2, 1) * other(0, 2) + at(3, 1) * other(0, 3),
				at(0, 2) * other(0, 0) + at(1, 2) * other(0, 1) + at(2, 2) * other(0, 2) + at(3, 2) * other(0, 3),
				at(0, 3) * other(0, 0) + at(1, 3) * other(0, 1) + at(2, 3) * other(0, 2) + at(3, 3) * other(0, 3),
			},
			{
				at(0, 0) * other(1, 0) + at(1, 0) * other(1, 1) + at(2, 0) * other(1, 2) + at(3, 0) * other(1, 3),
				at(0, 1) * other(1, 0) + at(1, 1) * other(1, 1) + at(2, 1) * other(1, 2) + at(3, 1) * other(1, 3),
				at(0, 2) * other(1, 0) + at(1, 2) * other(1, 1) + at(2, 2) * other(1, 2) + at(3, 2) * other(1, 3),
				at(0, 3) * other(1, 0) + at(1, 3) * other(1, 1) + at(2, 3) * other(1, 2) + at(3, 3) * other(1, 3),
			},
			{
				at(0, 0) * other(2, 0) + at(1, 0) * other(2, 1) + at(2, 0) * other(2, 2) + at(3, 0) * other(2, 3),
				at(0, 1) * other(2, 0) + at(1, 1) * other(2, 1) + at(2, 1) * other(2, 2) + at(3, 1) * other(2, 3),
				at(0, 2) * other(2, 0) + at(1, 2) * other(2, 1) + at(2, 2) * other(2, 2) + at(3, 2) * other(2, 3),
				at(0, 3) * other(2, 0) + at(1, 3) * other(2, 1) + at(2, 3) * other(2, 2) + at(3, 3) * other(2, 3),
			},
			{
				at(0, 0) * other(3, 0) + at(1, 0) * other(3, 1) + at(2, 0) * other(3, 2) + at(3, 0) * other(3, 3),
				at(0, 1) * other(3, 0) + at(1, 1) * other(3, 1) + at(2, 1) * other(3, 2) + at(3, 1) * other(3, 3),
				at(0, 2) * other(3, 0) + at(1, 2) * other(3, 1) + at(2, 2) * other(3, 2) + at(3, 2) * other(3, 3),
				at(0, 3) * other(3, 0) + at(1, 3) * other(3, 1) + at(2, 3) * other(3, 2) + at(3, 3) * other(3, 3),
			},
		};
	}

	[[nodiscard]] constexpr Vec4<T> operator*(const Vec4<T>& v) const noexcept {
		return {
			at(0, 0) * v[0] + at(1, 0) * v[1] + at(2, 0) * v[2] + at(3, 0) * v[3],
			at(0, 1) * v[0] + at(1, 1) * v[1] + at(2, 1) * v[2] + at(3, 1) * v[3],
			at(0, 2) * v[0] + at(1, 2) * v[1] + at(2, 2) * v[2] + at(3, 2) * v[3],
			at(0, 3) * v[0] + at(1, 3) * v[1] + at(2, 3) * v[2] + at(3, 3) * v[3],
		};
	}

	[[nodiscard]] constexpr Matrix<T, 4, 4> hadamard(const Matrix<T, 4, 4>& other) const noexcept {
		return {x * other.x, y * other.y, z * other.z, w * other.w};
	}

	[[nodiscard]] constexpr T determinant() const noexcept {
		return {at(0, 0) * at(1, 1) * at(2, 2) * at(3, 3) - at(3, 0) * at(2, 1) * at(1, 2) * at(0, 3) +
				at(1, 0) * at(2, 1) * at(3, 2) * at(0, 3) - at(2, 0) * at(1, 1) * at(0, 2) * at(3, 3) +
				at(2, 0) * at(3, 1) * at(0, 2) * at(1, 3) - at(1, 0) * at(0, 1) * at(3, 2) * at(2, 3) +
				at(3, 0) * at(0, 1) * at(1, 2) * at(2, 3) - at(0, 0) * at(3, 1) * at(2, 2) * at(1, 3)};
	}
	[[nodiscard]] constexpr Vec4<T> row(usize index) const noexcept {
		debug_assert(index < 4);
		return {x[index], y[index], z[index], w[index]};
	}
	[[nodiscard]] constexpr Matrix<T, 4, 4> transposed() const noexcept {
		return {row(0), row(1), row(2), row(3)};
	}
	constexpr Matrix<T, 4, 4>& transpose() noexcept {
		*this = transposed();
		return *this;
	}
};

template <typename T> using Mat4 = Matrix<T, 4, 4>;
using Mat4f = Mat4<f32>;

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

template <typename T> struct Quaternion {
	T r, i, j, k;

	constexpr Quaternion(const T r = 0, const T i = 0, const T j = 0, const T k = 0) noexcept
		: r(r), i(i), j(j), k(k) {}

	[[nodiscard]] constexpr T& operator[](usize index) noexcept {
		debug_assert(index < 4);
		return *((T*)this + index);
	}
	[[nodiscard]] constexpr const T& operator[](usize index) const noexcept {
		debug_assert(index < 4);
		return *((T*)this + index);
	}

	[[nodiscard]] constexpr static Quaternion from_axis_angle(const Vec3<T>& axis, const T radians) noexcept {
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
		return {result.i, result.j, result.k};
	}
	[[nodiscard]] constexpr Mat3<T> operator*(const Mat3<T>& v) const noexcept {
		return {*this * v.x, *this * v.y, *this * v.z};
	}

	[[nodiscard]] constexpr Quaternion operator+(const Quaternion& other) const noexcept {
		return {r + other.r, i + other.i, j + other.j, k + other.k};
	}
	[[nodiscard]] constexpr Quaternion operator-(const Quaternion& other) const noexcept {
		return {r - other.r, i - other.i, j - other.j, k - other.k};
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
		return {-r, -i, -j, -k};
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
		return {r + scalar, i, j, k};
	}
	[[nodiscard]] constexpr Quaternion operator-(const T scalar) const noexcept {
		return {r - scalar, i, j, k};
	}

	[[nodiscard]] constexpr Quaternion operator*(const T scalar) const noexcept {
		return {r * scalar, i * scalar, j * scalar, k * scalar};
	}
	[[nodiscard]] constexpr Quaternion operator/(const T scalar) const noexcept {
		debug_assert(scalar != 0);
		return {r / scalar, i / scalar, j / scalar, k / scalar};
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
		debug_assert(scalar != 0);
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
		return abs_squared() < other.abs_squared();
	}
	[[nodiscard]] constexpr bool operator<=(const Quaternion& other) const noexcept {
		return abs_squared() <= other.abs_squared();
	}
	[[nodiscard]] constexpr bool operator>(const Quaternion& other) const noexcept {
		return abs_squared() > other.abs_squared();
	}
	[[nodiscard]] constexpr bool operator>=(const Quaternion& other) const noexcept {
		return abs_squared() >= other.abs_squared();
	}

	[[nodiscard]] constexpr T dot(const Quaternion& other) const noexcept {
		return r * other.r + i * other.i + j * other.j + k * other.k;
	}
	[[nodiscard]] constexpr T abs_squared() const noexcept {
		return dot(*this);
	}
	[[nodiscard]] constexpr T abs() const noexcept {
		return std::sqrt(abs_squared());
	}
	[[nodiscard]] constexpr Quaternion normalized() const noexcept {
		static_assert(r != 0 && i != 0 && j != 0 && k != 0, "cannot divide quaterion by 0");
		return *this / abs();
	}
	constexpr Quaternion& normalize() noexcept {
		*this = normalized();
		return *this;
	}
	[[nodiscard]] constexpr Quaternion conjugate() const noexcept {
		return {r, -i, -j, -k};
	}
};

template <typename T> using Quat = Quaternion<T>;
using Quatf = Quaternion<f32>;

template <typename T> struct Transform2D {
	Vec3<T> position = {0};
	Vec2<T> scale = {1};
	T radians = 0;

	[[nodiscard]] constexpr Mat4<T> matrix() const noexcept {
		Mat2<T> m2{1};
		m2.x.x = scale.x;
		m2.y.y = scale.y;
		Mat2<T> rot = {cos(radians), sin(radians), -sin(radians), cos(radians)};
		m2 = rot * m2;
		Mat4<T> m4{m2};
		m4.w.x = position.x;
		m4.w.y = position.y;
		m4.w.z = position.z;
		return m4;
	}

	constexpr Transform2D& translate(const Vec2<T>& delta) noexcept {
		position += delta;
		return *this;
	}
	constexpr Transform2D& rotate(const T angle_radians) noexcept {
		radians += angle_radians;
		return *this;
	}
};

using Transform2Df = Transform2D<f32>;

template <typename T> struct Transform3D {
	Vec3<T> position = {0};
	Vec3<T> scale = {1};
	Quaternion<T> rotation = {1};

	[[nodiscard]] constexpr Mat4<T> matrix() const noexcept {
		Mat3<T> m3{1};
		m3.x.x = scale.x;
		m3.y.y = scale.y;
		m3.z.z = scale.z;
		m3 = rotation * m3;
		Mat4<T> m4{m3};
		m4.w.x = position.x;
		m4.w.y = position.y;
		m4.w.z = position.z;
		return m4;
	}

	constexpr Transform3D& translate(const Vec3<T>& delta) noexcept {
		position += delta;
		return *this;
	}
	constexpr Transform3D& rotate_external(const Quaternion<T>& delta) noexcept {
		rotation = delta * rotation;
		return *this;
	}
	constexpr void rotate_internal(const Quaternion<T>& delta) noexcept {
		rotation = rotation * delta;
		return *this;
	}
};

using Transform3Df = Transform3D<f32>;

template <typename T> struct Camera {
	Vec3<T> position{0};
	Quaternion<T> rotation{1};

	[[nodiscard]] constexpr Mat4<T> view() const noexcept {
		Mat4<T> rot{rotation.conjugate() * Mat3<T>{1}};
		Mat4<T> pos{1};
		pos.w.x = -position.x;
		pos.w.y = -position.y;
		pos.w.z = -position.z;
		return rot * pos;
	}

	constexpr void translate(const Vec3<T> delta) noexcept {
		position += delta;
	}
	constexpr void move(const Vec3<T> dir, T distance) noexcept {
		Vec3<T> d = rotation * Vec3<T>{dir.x, 0, dir.z};
		d.y = dir.y;
		position += d.normalized() * distance;
	}
	constexpr void rotate_external(const Quaternion<T> delta) noexcept {
		rotation = delta * rotation;
	}
	constexpr void rotate_internal(const Quaternion<T> delta) noexcept {
		rotation = rotation * delta;
	}

	[[nodiscard]] constexpr static Mat4<T> orthographic(Vec3<T> origin, Vec3<T> size) noexcept {
		return {
			{1 / size.x, 0, 0, 0}, {0, 1 / size.y, 0, 0}, {0, 0, 1 / size.z, 0}, {-origin.x, -origin.y, -origin.z, 1}};
	}

	[[nodiscard]] constexpr static Mat4<T> perspective(T near, T far, T aspect_ratio, T fov_radians) noexcept {
		T scale = 1 / std::tan(fov_radians);
		return Mat4<T>{{scale / aspect_ratio, 0, 0, 0},
					   {0, scale, 0, 0},
					   {0, 0, far / (far - near), 1},
					   {0, 0, -(far * near) / (far - near), 0}};
	}
};

using Cameraf = Camera<f32>;

} // namespace Mirror
