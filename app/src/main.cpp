#include <print>

#include "mirror.h"

#include <SDL3/SDL.h>

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	auto window = SDL_CreateWindow("Mirror", 1920, 1080, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	SDL_DestroyWindow(window);
	SDL_Quit();
}
