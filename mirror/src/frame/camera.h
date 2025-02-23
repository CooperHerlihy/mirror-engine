#pragma once

#include "types.h"
#include "matrix.h"
#include "vector.h"
#include "complex.h"
#include "quaternion.h"

namespace Mirror {

template<typename T>
struct Camera {
	Vec3<T> position{ 0 };
	Quaternion<T> rotation{ 1 };

	[[nodiscard]] constexpr Mat4<T> view() const noexcept {
		Mat4<T> rot{ rotation.conjugate() * Mat3<T>{ 1 } };
		Mat4<T> pos{ 1 };
		pos.w.x = -position.x;
		pos.w.y = -position.y;
		pos.w.z = -position.z;
		return rot * pos;
	}

	constexpr void translate(const Vec3<T> delta) noexcept { position += delta; }
	constexpr void move(const Vec3<T> dir, T distance) noexcept { 
		Vec3<T> d = rotation * Vec3<T>{ dir.x, 0, dir.z };
		d.y = dir.y;
		position += d.normalized() * distance;
	}
	constexpr void rotateExternal(const Quaternion<T> delta) noexcept { rotation = delta * rotation; }
	constexpr void rotateInternal(const Quaternion<T> delta) noexcept { rotation = rotation * delta; }

	[[nodiscard]] constexpr static Mat4<T> orthographic(Vec3<T> origin, Vec3<T> size) noexcept {
		return {
			{ size.y / size.x, 0, 0, 0 },
			{ 0, 1 / size.y, 0, 0 },
			{ 0, 0, 1 / size.z, 0 },
			{ -origin.x, -origin.y, -origin.z, 1 }
		};
	}

	[[nodiscard]] constexpr static Mat4<T> perspective(T near, T far, T aspect_ratio, T fov_radians) noexcept {
		T scale = 1 / std::tan(fov_radians);
		Mat4<T> proj{
			{ scale / aspect_ratio, 0, 0, 0 },
			{ 0, scale, 0, 0 },
			{ 0, 0, far / (far - near), 1 },
			{ 0, 0, -(far * near) / (far - near), 0 }
		};
		return proj;
	}

};

using Cameraf = Camera<f32>;

}
