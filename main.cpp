#include <SDL2/SDL_blendmode.h>
#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <CL/opencl.hpp>
#include <SDL2/SDL.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <stdexcept>

int WIDTH = 640;
int HEIGHT = 480;

void checkError(cl_int err, std::string target) {
	if (err != 0) {
		throw std::runtime_error("Error caught at " + target + ": " + std::to_string(err));
	}
}

int main() {

	cl::Platform platform = cl::Platform::get();
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
	cl::Device device = devices.front();

	auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
	auto name = device.getInfo<CL_DEVICE_NAME>();
	auto version = device.getInfo<CL_DEVICE_VERSION>();
	std::cout << vendor << ", " << name << ", " << version << std::endl;

	cl::Context context(device);

	std::ifstream vertexFile("../src/opencl/vertex.cl");
	std::string vertexSrc(std::istreambuf_iterator<char>(vertexFile),
			(std::istreambuf_iterator<char>()));

	cl_int err;
	cl::Program vertexProgram(context, vertexSrc.c_str(), CL_TRUE, &err);
	checkError(err, "Vertex Program");

	cl::CommandQueue queue(context, device, 0, &err);
	checkError(err, "Queue");

	/* x, y, z */
	float points[] = {
		-0.5f, -0.5f, -15.f,
		0.5f, -0.5f, -15.0f,
		0.0f, 0.5f, -15.0f,
		0.0f, 0.0f, -15.0f
	};

	int pointsCount = sizeof(points) / sizeof(float);

	cl::EnqueueArgs pointArgs(queue, cl::NDRange(pointsCount));

	cl::Buffer pointsBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			pointsCount*sizeof(points), points, &err);
	checkError(err, "PointsBuffer");

	float pointsOut[pointsCount];
	cl::Buffer pointsOutBuffer(context, CL_MEM_READ_WRITE, sizeof(pointsOut));
	checkError(err, "PointsOutBuffer");

	cl::Kernel perspectiveMultKernel(vertexProgram, "perspectiveMult", &err);
	checkError(err, "PerspectiveMult");
	cl::KernelFunctor<> perspectiveMult(perspectiveMultKernel);

	err = perspectiveMultKernel.setArg(0, sizeof(cl_mem), &pointsBuffer);
	checkError(err, "PerspectiveMult Arg 0");

	err = perspectiveMultKernel.setArg(1, pointsOutBuffer);
	checkError(err, "PerspectiveMult Arg 1");

	perspectiveMult(pointArgs);

	cl::Kernel centerFlipYKernel(vertexProgram, "centerFlipY", &err);
	checkError(err, "CenterFlipY Kernel");
	cl::KernelFunctor<> centerFlipY(centerFlipYKernel);

	err = centerFlipYKernel.setArg(0, sizeof(cl_mem), &pointsOutBuffer);
	checkError(err, "CenterFlipYKernel Arg 0");

	err = centerFlipYKernel.setArg(1, pointsOutBuffer);
	checkError(err, "CenterFlipYKernel Arg 1");

	err = centerFlipYKernel.setArg(2, sizeof(int), &WIDTH);
	checkError(err, "CenterFlipYKernel Arg 2");

	err = centerFlipYKernel.setArg(3, sizeof(int), &HEIGHT);
	checkError(err, "CenterFlipYKernel Arg 3");

	centerFlipY(pointArgs);

	cl::Kernel drawPointsKernel(vertexProgram, "drawPoints", &err);
	checkError(err, "DrawPointsKernel");
	cl::KernelFunctor<> drawPoints(drawPointsKernel);

	err = drawPointsKernel.setArg(0, sizeof(cl_mem), &pointsOutBuffer);
	checkError(err, "drawPointsKernel Arg 0");

	uint32_t textureBuffer[WIDTH*HEIGHT];
	cl::Buffer pixelBuffer(context, CL_MEM_WRITE_ONLY, sizeof(textureBuffer));
	err = drawPointsKernel.setArg(1, pixelBuffer);
	checkError(err, "drawPointsKernel Arg 1");

	err = drawPointsKernel.setArg(2, sizeof(int), &WIDTH);
	checkError(err, "drawPointsKernel Arg 2");

	drawPoints(pointArgs);

	queue.enqueueReadBuffer(pixelBuffer, CL_TRUE, 0, sizeof(textureBuffer),
			textureBuffer);

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window;
	window = SDL_CreateWindow(
			"OpenCL Graphics",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			WIDTH,
			HEIGHT,
			SDL_WINDOW_RESIZABLE);

	if (window == NULL) {
		throw std::runtime_error("Failed to create window");
	}

	SDL_Event event;
	int running = 1;

	SDL_Renderer* renderer;
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
			SDL_RENDERER_PRESENTVSYNC);

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

	SDL_UpdateTexture(texture, NULL, textureBuffer, WIDTH*sizeof(uint32_t));
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
}
