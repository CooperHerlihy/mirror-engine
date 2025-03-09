#pragma once

#include "types.h"
#include "matrix.h"
#include "vector.h"
#include "complex.h"
#include "quaternion.h"

namespace Mirror {

template<typename T>
struct Transform3D {
	Vec3<T> position = { 0 };
	Vec3<T> scale = { 1 };
	Quaternion<T> rotation = { 1 };

	[[nodiscard]] constexpr Mat4<T> matrix() const noexcept {
		Mat3<T> m3{ 1 };
		m3.x.x = scale.x;
		m3.y.y = scale.y;
		m3.z.z = scale.z;
		m3 = rotation * m3;
		Mat4<T> m4{ m3 };
		m4.w.x = position.x;
		m4.w.y = position.y;
		m4.w.z = position.z;
		return m4;
	}

	constexpr void translate(const Vec3<T>& delta) noexcept {
		position += delta;
	}
	constexpr void rotateExternal(const Quaternion<T>& delta) noexcept {
		rotation = delta * rotation;
	}
	constexpr void rotateInternal(const Quaternion<T>& delta) noexcept {
		rotation = rotation * delta;
	}
};

using Transform3Df = Transform3D<f32>;

template<typename T>
struct Transform2D {
	Vec3<T> position = { 0 };
	Vec2<T> scale = { 1 };
	T radians = 0;

	[[nodiscard]] constexpr Mat4<T> matrix() const noexcept {
		Mat2<T> m2{ 1 };
		m2.x.x = scale.x;
		m2.y.y = scale.y;
		m2 = { cos(radians), sin(radians), -sin(radians), cos(radians) } * m2;
		Mat4<T> m4{ m2 };
		m4.w.x = position.x;
		m4.w.y = position.y;
		m4.w.z = position.z;
		return m4;
	}

	constexpr void translate(const Vec2<T>& delta) noexcept {
		position += delta;
	}
	constexpr void rotate(const T angle_radians) noexcept {
		radians += angle_radians;
	}
};

using Transform2Df = Transform2D<f32>;

}
