#include <print>

#include <SDL3/SDL.h>

#include "mirror/mirror.h"

struct Input {
	enum Direction : u8 {
		Up = 1 << 0,
		Down = 1 << 1,
		Left = 1 << 2,
		Right = 1 << 3,
		Forward = 1 << 4,
		Back = 1 << 5
	};

	u8 bits = 0;

	constexpr void set(Direction dir, bool value) noexcept {
		if (value) {
			set(dir);
		} else {
			reset(dir);
		}
	}
	constexpr void set(Direction dir) noexcept {
		bits |= dir;
	}
	constexpr void reset(Direction dir) noexcept {
		bits &= ~dir;
	}

	[[nodiscard]] constexpr bool get(Direction dir) const noexcept {
		return bits & dir;
	}
};

int main() {
	Mirror::Vk::FurnaceKeeper furnace{"Mirror App", "1.0.0"};

	Mirror::Reflect::Renderer renderer{"Mirror Window", {1920, 1080}};
	Mirror::Cameraf camera = {.position = {0.0f, 0.0f, -1.0f}};
	renderer.set_view(camera.view());
	f32 camera_fov = 3.1415926535f / 4.0f;
	renderer.set_projection(Mirror::Cameraf::perspective(0.01f, 100.0f, 1920.0f / 1080.0f, camera_fov));

	auto cat_texture = renderer.load_sprite_texture("../assets/cat.png");
	Mirror::Mat2f cat_tex_coords = {{0.0f, 0.0f}, {1.0f, 1.0f}};

	Mirror::Transform3Df cat_transform;
	cat_transform.scale.x = 1.5f;

	Mirror::Transform3Df circling_cat;
	circling_cat.position = {0.0f, 0.0f, 1.0f};
	circling_cat.scale.x = 1.5f;

	Input movement;

	Mirror::Clock clock{};
	f64 print_timer = 0;
	i32 frame_count = 0;
	while (true) {
		clock.update();
		f64 delta = clock.delta_sec();

		if (print_timer >= 1.0) {
			std::println("avg: {}ms", 1'000.0 / frame_count);
			print_timer -= 1.0;
			frame_count = 0;
		}
		print_timer += delta;
		frame_count++;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_EVENT_QUIT:
				exit(0);
			case SDL_EVENT_WINDOW_MINIMIZED:
				for (; event.type != SDL_EVENT_WINDOW_RESTORED; SDL_WaitEvent(&event)) {}
				break;
			case SDL_EVENT_WINDOW_RESIZED: {
				for (bool success = renderer.resize(); !success; success = renderer.resize()) {
					for (SDL_WaitEvent(&event); event.type != SDL_EVENT_WINDOW_RESIZED; SDL_WaitEvent(&event)) {}
				}
				break;
			}
			case SDL_EVENT_MOUSE_MOTION:
				if (event.motion.state & SDL_BUTTON_LMASK) {
					constexpr f64 sensitivity = 0.006f;
					camera.rotate_external(Mirror::Quatf::from_axis_angle(
						Mirror::Vec3f{0.0f, 1.0f, 0.0f}, static_cast<f32>(event.motion.xrel * sensitivity)));
					camera.rotate_internal(Mirror::Quatf::from_axis_angle(
						Mirror::Vec3f{1.0f, 0.0f, 0.0f}, static_cast<f32>(-event.motion.yrel * sensitivity)));
					renderer.set_view(camera.view());
				}
				break;
			case SDL_EVENT_MOUSE_WHEEL:
				camera_fov -= event.wheel.y * 0.01f;
				renderer.set_projection(Mirror::Cameraf::perspective(0.01f, 100.0f, 1920.0f / 1080.0f, camera_fov));
				break;
			case SDL_EVENT_KEY_DOWN:
				switch (event.key.key) {
				case SDLK_SPACE:
					movement.set(Input::Direction::Up);
					break;
				case SDLK_LSHIFT:
					movement.set(Input::Direction::Down);
					break;
				case SDLK_W:
					movement.set(Input::Direction::Forward);
					break;
				case SDLK_A:
					movement.set(Input::Direction::Left);
					break;
				case SDLK_S:
					movement.set(Input::Direction::Back);
					break;
				case SDLK_D:
					movement.set(Input::Direction::Right);
					break;
				}
				break;
			case SDL_EVENT_KEY_UP:
				switch (event.key.key) {
				case SDLK_SPACE:
					movement.reset(Input::Direction::Up);
					break;
				case SDLK_LSHIFT:
					movement.reset(Input::Direction::Down);
					break;
				case SDLK_W:
					movement.reset(Input::Direction::Forward);
					break;
				case SDLK_A:
					movement.reset(Input::Direction::Left);
					break;
				case SDLK_S:
					movement.reset(Input::Direction::Back);
					break;
				case SDLK_D:
					movement.reset(Input::Direction::Right);
					break;
				}
				break;
			}
		}

		constexpr f32 cam_speed = 2.0f;
		Mirror::Vec3f velocity = {
			static_cast<f32>(movement.get(Input::Direction::Right) - movement.get(Input::Direction::Left)),
			static_cast<f32>(movement.get(Input::Direction::Down) - movement.get(Input::Direction::Up)),
			static_cast<f32>(movement.get(Input::Direction::Forward) - movement.get(Input::Direction::Back)),
		};
		if (velocity.x != 0 || velocity.y != 0 || velocity.z != 0) {
			camera.move(Mirror::Vec3f{velocity.x, velocity.y, velocity.z}.normalized(),
						cam_speed * static_cast<f32>(delta));
			renderer.set_view(camera.view());
		}

		renderer.render_sprite(cat_texture, {cat_transform.matrix(), cat_tex_coords});

		auto rot = Mirror::Quatf::from_axis_angle({0.0f, 1.0f, 0.0f}, static_cast<f32>(delta * 4.0));
		circling_cat.position = rot * circling_cat.position;
		circling_cat.rotate_external(rot);
		renderer.render_sprite(cat_texture, {circling_cat.matrix(), cat_tex_coords});

		renderer.update();
	}
}
