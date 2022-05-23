#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_timer.h>
#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <CL/opencl.hpp>
#include <SDL2/SDL.h>
#include "src/clg.hpp"

#include <iostream>
#include <math.h>

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;
float FOV = 90.0f;

int main() {

	clg clg(SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow(
			"OpenCL Graphics",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			SCREEN_WIDTH,
			SCREEN_HEIGHT,
			SDL_WINDOW_RESIZABLE);

	if (window == NULL) {
		throw std::runtime_error("Failed to create window");
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_Texture* screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetTextureBlendMode(screen, SDL_BLENDMODE_BLEND);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	SDL_Event event;
	int running = 1;

	float projMat[16];
	createProjMat(60.0f, 0.1f, 1000.0f, projMat);
	float scaleMat[16];
	createScaleMat(5.f, scaleMat);
	float transMat[16];
	createTransMat(0.0f, 0.0f, -50.0f, transMat);

	float vertices[] {
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,

		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,

		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
	};

	/*float vertices[] = {
		-1.0f, -1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	-1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	1.0, 1.0,
		0.0f, 1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	1.0, -1.0,

		0.0f, 1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	-1.0f, -1.0f,
		2.0f, 1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	1.0f, -1.0f
	};*/

	/*float vertices[] = {
		-1.0f, -1.0f, 0.0f, 	0.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 	0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 	0.0f, 0.0f, 1.0f,

		0.0f, 1.0f, 0.0f, 	0.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 	0.0f, 0.0f, 1.0f,
		2.0f, 1.0f, 0.0f, 	0.0f, 0.0f, 1.0f
	};*/

	int pointCount = sizeof(vertices) / sizeof(float);

	vec3 cameraPos(0.0f, 0.0f, 10.0f);
	vec3 front(0.0f, 0.0f, -1.0f);
	vec3 up(0.0f, 1.0f, 0.0f);
	float cameraMovementSpeed = 50.0f;

	uint64_t now = SDL_GetPerformanceCounter();
	uint64_t last = 0;
	double deltaTime = 0;

	int x,y;

	float yaw = -1.5f;
	float pitch = 0.0f;

	float rot = 0.0f;
	while (running) {
		last = now;
		now = SDL_GetPerformanceCounter();
		deltaTime = (double)((now - last)*1000 /
				(double)SDL_GetPerformanceFrequency()) * 0.001f;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
				running = 0;
			}
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				SCREEN_WIDTH = event.window.data1;
				SCREEN_HEIGHT = event.window.data2;
				clg.setScreenWidthHeight(SCREEN_WIDTH, SCREEN_HEIGHT);
				clg.updateScreen(NULL);
				SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
				SDL_RenderSetViewport(renderer, NULL);
				screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
						SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
			}
		}

		const uint8_t* keystate = SDL_GetKeyboardState(NULL);
		SDL_GetRelativeMouseState(&x, &y);

		yaw += (float)x/400.0f;
		pitch -= (float)y/400.0f;
		if (pitch > 1.49f) {
			pitch = 1.49f;
		}
		if (pitch < -1.49f) {
			pitch = -1.49f;
		}

		vec3 direction(cosf(yaw) * cosf(pitch), sinf(pitch), sinf(yaw) * cosf(pitch));
		front = direction;

		if (keystate[SDL_SCANCODE_W]) {
			cameraPos.x += front.x * cameraMovementSpeed * deltaTime;
			cameraPos.y += front.y * cameraMovementSpeed * deltaTime;
			cameraPos.z += front.z * cameraMovementSpeed * deltaTime;
		}

		if (keystate[SDL_SCANCODE_S]) {
			cameraPos.x -= front.x * cameraMovementSpeed * deltaTime;
			cameraPos.y -= front.y * cameraMovementSpeed * deltaTime;
			cameraPos.z -= front.z * cameraMovementSpeed * deltaTime;
		}

		if (keystate[SDL_SCANCODE_D]) {
			cameraPos.x += normalize(crossVec(front, up)).x *
				cameraMovementSpeed * deltaTime;
			cameraPos.y += normalize(crossVec(front, up)).y *
				cameraMovementSpeed * deltaTime;
			cameraPos.z += normalize(crossVec(front, up)).z *
				cameraMovementSpeed * deltaTime;
		}

		if (keystate[SDL_SCANCODE_A]) {
			cameraPos.x -= normalize(crossVec(front, up)).x *
				cameraMovementSpeed * deltaTime;
			cameraPos.y -= normalize(crossVec(front, up)).y *
				cameraMovementSpeed * deltaTime;
			cameraPos.z -= normalize(crossVec(front, up)).z *
				cameraMovementSpeed * deltaTime;
		}

		if (keystate[SDL_SCANCODE_SPACE]) {
			cameraPos.y += cameraMovementSpeed * deltaTime;
		}

		if (keystate[SDL_SCANCODE_LSHIFT]) {
			cameraPos.y -= cameraMovementSpeed * deltaTime;
		}

		float rotMat[16];
		createRotMat(0.0f, rot, rot, rotMat);
		rot += 0.5f;

		vec3 target(cameraPos.x+front.x, cameraPos.y+front.y, cameraPos.z+front.z);
		float viewMat[16];
		createLookAtMat(cameraPos, target, up, viewMat);

		int screenBuffer[SCREEN_WIDTH*SCREEN_HEIGHT];

		clg.drawWireframeDots(vertices, 3, pointCount, 255, 0, 255,
				255, 255, 255, 3, scaleMat, rotMat, transMat, viewMat, projMat, true);

		clg.updateScreen(screenBuffer);

		SDL_UpdateTexture(screen, NULL, screenBuffer, SCREEN_WIDTH*sizeof(int));
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, screen, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
}
