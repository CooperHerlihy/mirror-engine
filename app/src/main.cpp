#include <print>

#include <SDL3/SDL.h>

#include "mirror.h"

int main() {
	if (!SDL_SetAppMetadata("Mirror App", "1.0.0", nullptr)) std::terminate();
	if (!SDL_Init(SDL_INIT_VIDEO)) std::terminate();

	Mirror::Mirror engine{ { 1920, 1080 }, "Mirror App" };

	bool running = true;
	while (running) {

		// Handle input
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_EVENT_QUIT:
				running = false;
				break;
			}
		}

		// Update game state

		// Update engine
		engine.update();
	}
}
