#pragma once

#include "types.h"

#include "vector.h"

namespace Mirror {

template<typename T, usize Rows, usize Cols>
struct Matrix {};

template<typename T>
struct Matrix<T, 2, 2> {
	Vec2<T> x, y;

	constexpr Matrix() noexcept : x(0), y(0) {}
	constexpr Matrix(const T scalar) noexcept : 
		x(scalar, 0),
		y(0, scalar)
	{}
	constexpr Matrix(const Vec2<T>& x, const Vec2<T>& y) noexcept : x(x), y(y) {}

	[[nodiscard]] constexpr Vec2<T>& operator[](usize col) noexcept {
		assert(col < 2);
		return *((Vec2<T>*)this + col);
	}
	[[nodiscard]] constexpr const Vec2<T>& operator[](usize col) const noexcept {
		assert(col < 2);
		return *((Vec2<T>*)this + col);
	}
	[[nodiscard]] constexpr T& operator()(usize col, usize row) noexcept {
		assert(col < 2 && row < 2);
		return *((T*)((Vec2<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr const T& operator()(usize col, usize row) const noexcept {
		assert(col < 2 && row < 2);
		return *((T*)((Vec2<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr T& at(usize col, usize row) noexcept {
		assert(col < 2 && row < 2);
		return operator()(col, row);
	}
	[[nodiscard]] constexpr const T& at(usize col, usize row) const noexcept {
		assert(col < 2 && row < 2);
		return operator()(col, row);
	}

	[[nodiscard]] constexpr Vec2<T> operator+(const Matrix<T, 2, 2>& other) const noexcept {
		return { x + other.x, y + other.y };
	}
	[[nodiscard]] constexpr Vec2<T> operator-(const Matrix<T, 2, 2>& other) const noexcept {
		return { x - other.x, y - other.y };
	}
	constexpr Vec2<T>& operator+=(const Matrix<T, 2, 2>& other) noexcept {
		return { x += other.x, y += other.y };
	}
	constexpr Vec2<T>& operator-=(const Matrix<T, 2, 2>& other) noexcept {
		return { x -= other.x, y -= other.y };
	}

	[[nodiscard]] constexpr Vec2<T> operator*(const T scalar) const noexcept {
		return { x * scalar, y * scalar };
	}
	[[nodiscard]] constexpr Vec2<T> operator/(const T scalar) const noexcept {
		assert(scalar != 0);
		return { x / scalar, y / scalar };
	}
	[[nodiscard]] constexpr Vec2<T>& operator*=(const T scalar) noexcept {
		return { x *= scalar, y *= scalar };
	}
	[[nodiscard]] constexpr Vec2<T>& operator/=(const T scalar) noexcept {
		assert(scalar != 0);
		return { x /= scalar, y /= scalar };
	}

	[[nodiscard]] constexpr Matrix<T, 2, 2> operator*(const Matrix<T, 2, 2>& other) const noexcept {
		return {
			{ at(0, 0) * other(0, 0) + at(1, 0) * other(0, 1), at(0, 1) * other(0, 0) + at(1, 1) * other(0, 1), },
			{ at(0, 0) * other(1, 0) + at(1, 0) * other(1, 1), at(0, 1) * other(1, 0) + at(1, 1) * other(1, 1), },
		};
	}

	[[nodiscard]] constexpr Vec2<T> operator*(const Vec2<T>& v) const noexcept {
		return { 
			at(0, 0) * v[0] + at(1, 0) * v[1], 
			at(0, 1) * v[0] + at(1, 1) * v[1],
		};
	}

	[[nodiscard]] constexpr Matrix<T, 2, 2> hadamard(const Matrix<T, 2, 2>& other) const noexcept {
		return { Vec2<T>{ x * other.x, y * other.y }, };
	}

	[[nodiscard]] constexpr T determinant() const noexcept {
		return { at(0, 0) * at(1, 1) - at(1, 0) * at(0, 1) };
	}
	[[nodiscard]] constexpr Vec2<T> row(usize index) const noexcept {
		assert(index < 2);
		return { x[index], y[index] };
	}
	[[nodiscard]] constexpr Matrix<T, 2, 2> transposed() const noexcept {
		return { row(0), row(1) };
	}
	constexpr Matrix<T, 2, 2>& transpose() noexcept {
		*this = transposed();
		return *this;
	}
};

template<typename T>
using Mat2 = Matrix<T, 2, 2>;
using Mat2f = Matrix<f32, 2, 2>;

template<typename T>
struct Matrix<T, 3, 3> {
	Vec3<T> x, y, z;

	constexpr Matrix() noexcept : x(0), y(0), z(0) {}
	constexpr Matrix(const T scalar) noexcept :
		x(scalar, 0, 0),
		y(0, scalar, 0),
		z(0, 0, scalar)
	{}
	constexpr Matrix(const Vec3<T>& x, const Vec3<T>& y, const Vec3<T>& z) noexcept : x(x), y(y), z(z) {}

	constexpr Matrix(const Mat2<T>& other) noexcept : x(other.x), y(other.y), z(0, 0, 1) {}

	[[nodiscard]] constexpr Vec3<T>& operator[](usize col) noexcept {
		assert(col < 3);
		return *((Vec3<T>*)this + col);
	}
	[[nodiscard]] constexpr const Vec3<T>& operator[](usize col) const noexcept {
		assert(col < 3);
		return *((Vec3<T>*)this + col);
	}
	[[nodiscard]] constexpr T& operator()(usize col, usize row) noexcept {
		assert(col < 3 && row < 3);
		return *((T*)((Vec3<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr const T& operator()(usize col, usize row) const noexcept {
		assert(col < 3 && row < 3);
		return *((T*)((Vec3<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr T& at(usize col, usize row) noexcept {
		assert(col < 3 && row < 3);
		return operator()(col, row);
	}
	[[nodiscard]] constexpr const T& at(usize col, usize row) const noexcept {
		assert(col < 3 && row < 3);
		return operator()(col, row);
	}

	[[nodiscard]] constexpr Vec3<T> operator+(const Matrix<T, 3, 3>& other) const noexcept {
		return { x + other.x, y + other.y, z + other.z };
	}
	[[nodiscard]] constexpr Vec3<T> operator-(const Matrix<T, 3, 3>& other) const noexcept {
		return { x - other.x, y - other.y, z - other.z };
	}
	constexpr Vec3<T>& operator+=(const Matrix<T, 3, 3>& other) noexcept {
		return { x += other.x, y += other.y, z += other.z };
	}
	constexpr Vec3<T>& operator-=(const Matrix<T, 3, 3>& other) noexcept {
		return { x -= other.x, y -= other.y, z -= other.z };
	}

	[[nodiscard]] constexpr Vec3<T> operator*(const T scalar) const noexcept {
		return { x * scalar, y * scalar, z * scalar };
	}
	[[nodiscard]] constexpr Vec3<T> operator/(const T scalar) const noexcept {
		assert(scalar != 0);
		return { x / scalar, y / scalar, z / scalar };
	}
	[[nodiscard]] constexpr Vec3<T>& operator*=(const T scalar) noexcept {
		return { x *= scalar, y *= scalar, z *= scalar };
	}
	[[nodiscard]] constexpr Vec3<T>& operator/=(const T scalar) noexcept {
		assert(scalar != 0);
		return { x /= scalar, y /= scalar, z /= scalar };
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
		return {
			Vec3<T>{
				x * other.x,
				y * other.y,
				z * other.z
			},
		};
	}

	[[nodiscard]] constexpr T determinant() const noexcept {
		return {
			at(0, 0) * at(1, 1) * at(2, 2) - at(2, 0) * at(1, 1) * at(0, 2) +
			at(1, 0) * at(2, 1) * at(0, 2) - at(1, 0) * at(0, 1) * at(2, 2) +
			at(2, 0) * at(0, 1) * at(1, 2) - at(0, 0) * at(2, 1) * at(1, 2)
		};
	}
	[[nodiscard]] constexpr Vec3<T> row(usize index) const noexcept {
		assert(index < 3);
		return { x[index], y[index], z[index] };
	}
	[[nodiscard]] constexpr Matrix<T, 3, 3> transposed() const noexcept {
		return { row(0), row(1), row(2) };
	}
	constexpr Matrix<T, 3, 3>& transpose() noexcept {
		*this = transposed();
		return *this;
	}
};

template<typename T>
using Mat3 = Matrix<T, 3, 3>;
using Mat3f = Matrix<f32, 3, 3>;

template<typename T>
struct Matrix<T, 4, 4> {
	Vec4<T> x, y, z, w;

	constexpr Matrix() noexcept : x(0), y(0), z(0), w(0) {}
	constexpr Matrix(const T scalar) noexcept : 
		x(scalar, 0, 0, 0),
		y(0, scalar, 0, 0),
		z(0, 0, scalar, 0),
		w(0, 0, 0, scalar)
	{}
	constexpr Matrix(const Vec4<T>& x, const Vec4<T>& y, const Vec4<T>& z, const Vec4<T>& w) noexcept : x(x), y(y), z(z), w(w) {}

	constexpr Matrix(const Mat2<T>& other) noexcept : x(other.x), y(other.y), z(0, 0, 1, 0), w(0, 0, 0, 1){}
	constexpr Matrix(const Mat3<T>& other) noexcept : x(other.x), y(other.y), z(other.z), w(0, 0, 0, 1){}

	[[nodiscard]] constexpr Vec4<T>& operator[](usize col) noexcept {
		assert(col < 4);
		return *((Vec4<T>*)this + col);
	}
	[[nodiscard]] constexpr const Vec4<T>& operator[](usize col) const noexcept {
		assert(col < 4);
		return *((Vec4<T>*)this + col);
	}
	[[nodiscard]] constexpr T& operator()(usize col, usize row) noexcept {
		assert(col < 4 && row < 4);
		return *((T*)((Vec4<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr const T& operator()(usize col, usize row) const noexcept {
		assert(col < 4 && row < 4);
		return *((T*)((Vec4<T>*)this + col) + row);
	}
	[[nodiscard]] constexpr T& at(usize col, usize row) noexcept {
		assert(col < 4 && row < 4);
		return operator()(col, row);
	}
	[[nodiscard]] constexpr const T& at(usize col, usize row) const noexcept {
		assert(col < 4 && row < 4);
		return operator()(col, row);
	}

	[[nodiscard]] constexpr Vec4<T> operator+(const Matrix<T, 4, 4>& other) const noexcept {
		return { x + other.x, y + other.y, z + other.z, w + other.w };
	}
	[[nodiscard]] constexpr Vec4<T> operator-(const Matrix<T, 4, 4>& other) const noexcept {
		return { x - other.x, y - other.y, z - other.z, w - other.w };
	}
	constexpr Vec4<T>& operator+=(const Matrix<T, 4, 4>& other) noexcept {
		return { x += other.x, y += other.y, z += other.z, w += other.w };
	}
	constexpr Vec4<T>& operator-=(const Matrix<T, 4, 4>& other) noexcept {
		return { x -= other.x, y -= other.y, z -= other.z, w -= other.w };
	}

	[[nodiscard]] constexpr Vec4<T> operator*(const T scalar) const noexcept {
		return { x * scalar, y * scalar, z * scalar, w * scalar };
	}
	[[nodiscard]] constexpr Vec4<T> operator/(const T scalar) const noexcept {
		assert(scalar != 0);
		return { x / scalar, y / scalar, z / scalar, w / scalar };
	}
	constexpr Vec4<T>& operator*=(const T scalar) noexcept {
		return { x *= scalar, y *= scalar, z *= scalar, w *= scalar };
	}
	constexpr Vec4<T>& operator/=(const T scalar) noexcept {
		assert(scalar != 0);
		return { x /= scalar, y /= scalar, z /= scalar, w /= scalar };
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
		return {
			Vec4<T>{
				x * other.x,
				y * other.y,
				z * other.z,
				w * other.w
			},
		};
	}

	[[nodiscard]] constexpr T determinant() const noexcept {
		return {
			at(0, 0) * at(1, 1) * at(2, 2) * at(3, 3) - at(3, 0) * at(2, 1) * at(1, 2) * at(0, 3) +
			at(1, 0) * at(2, 1) * at(3, 2) * at(0, 3) - at(2, 0) * at(1, 1) * at(0, 2) * at(3, 3) +
			at(2, 0) * at(3, 1) * at(0, 2) * at(1, 3) - at(1, 0) * at(0, 1) * at(3, 2) * at(2, 3) +
			at(3, 0) * at(0, 1) * at(1, 2) * at(2, 3) - at(0, 0) * at(3, 1) * at(2, 2) * at(1, 3)
		};
	}
	[[nodiscard]] constexpr Vec4<T> row(usize index) const noexcept {
		assert(index < 4);
		return {
			x[index],
			y[index],
			z[index],
			w[index]
		};
	}
	[[nodiscard]] constexpr Matrix<T, 4, 4> transposed() const noexcept {
		return {
			row(0),
			row(1),
			row(2),
			row(3)
		};
	}
	constexpr Matrix<T, 4, 4>& transpose() noexcept {
		*this = transposed();
		return *this;
	}
};

template<typename T>
using Mat4 = Matrix<T, 4, 4>;
using Mat4f = Mat4<f32>;

}

