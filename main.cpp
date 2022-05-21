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

struct vec3 {
	float x, y, z;
	vec3() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	vec3(float xx, float yy, float zz) {
		x = xx;
		y = yy;
		z = zz;
	}
};

// a - b
vec3 subVec(vec3 a, vec3 b) {
	vec3 ans;
	ans.x = a.x - b.x;
	ans.y = a.y - b.y;
	ans.z = a.z - b.z;
	return ans;
}

float dotVec(vec3 a, vec3 b) {
	return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
}

vec3 normalize(vec3 a) {
	vec3 ans;
	float l = sqrtf(dotVec(a, a));
	ans.x = a.x / l;
	ans.y = a.y / l;
	ans.z = a.z / l;
	return ans;
}

vec3 crossVec(vec3 a, vec3 b) {
	vec3 ans;
	ans.x = (a.y * b.z) - (a.z * b.y);
	ans.y = (a.z * b.x) - (a.x * b.z);
	ans.z = (a.x * b.y) - (a.y * b.x);
	return ans;
}

void createLookAtMat(vec3 pos, vec3 target, vec3 up, float* m) {
	vec3 zaxis = normalize(subVec(target, pos));
	vec3 xaxis = normalize(crossVec(zaxis, up));
	vec3 yaxis = crossVec(xaxis, zaxis);

	zaxis.x = -zaxis.x;
	zaxis.y = -zaxis.y;
	zaxis.z = -zaxis.z;

	m[0] = xaxis.x;
	m[1] = xaxis.y;
	m[2] = xaxis.z;
	m[3] = -dotVec(xaxis, pos);

	m[4] = yaxis.x;
	m[5] = yaxis.y;
	m[6] = yaxis.z;
	m[7] = -dotVec(yaxis, pos);

	m[8] = zaxis.x;
	m[9] = zaxis.y;
	m[10] = zaxis.z;
	m[11] = -dotVec(zaxis, pos);

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
}

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

	SDL_Event event;
	int running = 1;

	float projMat[16];
	createProjMat(60.0f, 0.1f, 1000.0f, projMat);
	float scaleMat[16];
	createScaleMat(5.f, scaleMat);
	float transMat[16];
	createTransMat(0.0f, 0.0f, -15.0f, transMat);

	/*float vertices[] {
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
	};*/

	float vertices[] = {
		-1.0f, -1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	-1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	1.0, 1.0,
		0.0f, 1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	1.0, -1.0,

		0.0f, 1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	-1.0f, -1.0f,
		2.0f, 1.0f, 0.0f, 	0.0f, 0.0f, 1.0f, 	1.0f, -1.0f
	};

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
	float cameraMovementSpeed = 20.0f;

	uint64_t now = SDL_GetPerformanceCounter();
	uint64_t last = 0;
	double deltaTime = 0;

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

		if (keystate[SDL_SCANCODE_W]) {
			cameraPos.z -= cameraMovementSpeed * deltaTime;
		}

		if (keystate[SDL_SCANCODE_S]) {
			cameraPos.z += cameraMovementSpeed * deltaTime;
		}

		if (keystate[SDL_SCANCODE_D]) {
			cameraPos.x += cameraMovementSpeed*2 * deltaTime;
		}

		if (keystate[SDL_SCANCODE_A]) {
			cameraPos.x -= cameraMovementSpeed*2 * deltaTime;
		}



		float rotMat[16];
		createRotMat(0.0f, rot, rot, rotMat);
		//rot += 0.5f;

		vec3 target(cameraPos.x+front.x, cameraPos.y+front.y, cameraPos.z+front.z);
		float viewMat[16];
		createLookAtMat(cameraPos, target, up, viewMat);

		int screenBuffer[SCREEN_WIDTH*SCREEN_HEIGHT];

		clg.drawWireframeDots(vertices, 8, pointCount, 255, 0, 255,
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
