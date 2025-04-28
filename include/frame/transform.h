#pragma once

#include "frame/complex.h"
#include "frame/matrix.h"
#include "frame/quaternion.h"
#include "frame/types.h"
#include "frame/vector.h"

namespace Mirror {

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

} // namespace Mirror
