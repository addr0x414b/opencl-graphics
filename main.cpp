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
		-0.5f, -0.5f, 0.f,
		-0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f
	};

	float radians = (80.f) * (3.141592f / 180.f);

	float rotYMat[4][4] = {
		{1.f, 0.0f, 0.0f, 0.0f},
		{0.0f, cosf(radians), -sinf(radians), 0.0f},
		{0.0f, sinf(radians), cosf(radians), 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
	};

	float*  rotYMatPtr = rotYMat[0];

	float fovRad = (60.f/2.f) * (3.141592f / 180.f);
	float aspect = (float)WIDTH / HEIGHT;
	float zNear = 0.1f;
	float zFar = 1000.f;

	float zz = (1.f / (tan(fovRad))) / aspect;
	float oo = 1.f / tan(fovRad);
	float tt = ((-2.f * zNear) / (zFar - zNear)) - 1.f;
	float tht = (-zNear * zFar) / (zFar - zNear);
	float tth = -1.0f;

	float projMat[4][4] = {
		{zz, 0.0f, 0.0f, 0.0f},
		{0.0f, oo, 0.0f, 0.0f},
		{0.0f, 0.0f, tt, tth},
		{0.0f, 0.0f, tht, 0.0f}
	};

	float* projMatPtr = projMat[0];

	int pointsCount = sizeof(points) / sizeof(float);

	cl::EnqueueArgs pointArgs(queue, cl::NDRange(pointsCount));

	cl::Buffer pointsBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			pointsCount*sizeof(float), points, &err);
	checkError(err, "PointsBuffer");

	float pointsOut[pointsCount];
	cl::Buffer pointsOutBuffer(context, CL_MEM_READ_WRITE, sizeof(pointsOut));
	checkError(err, "PointsOutBuffer");

	cl::Kernel multiplyPointsKernel(vertexProgram, "multiplyPoints", &err);
	checkError(err, "MultiplyPointsKernel");
	cl::KernelFunctor<> multiplyPoints(multiplyPointsKernel);

	cl::Buffer rotYMatBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			4*4*sizeof(float), rotYMatPtr, &err);
	checkError(err, "RotYMatBuffer");

	err = multiplyPointsKernel.setArg(0, sizeof(cl_mem), &pointsBuffer);
	checkError(err, "ProjMat Arg 0");

	err = multiplyPointsKernel.setArg(1, sizeof(cl_mem), &pointsOutBuffer);
	checkError(err, "ProjMat Arg 1");

	err = multiplyPointsKernel.setArg(2, sizeof(cl_mem), &rotYMatBuffer);
	checkError(err, "ProjMat Arg 2");

	multiplyPoints(pointArgs);

	cl::Buffer projMatBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			4*4*sizeof(float), projMatPtr, &err);
	checkError(err, "ProjMatBuffer");

	err = multiplyPointsKernel.setArg(0, sizeof(cl_mem), &pointsOutBuffer);
	checkError(err, "ProjMat Arg 0");

	err = multiplyPointsKernel.setArg(1, sizeof(cl_mem), &pointsOutBuffer);
	checkError(err, "ProjMat Arg 1");

	err = multiplyPointsKernel.setArg(2, sizeof(cl_mem), &projMatBuffer);
	checkError(err, "ProjMat Arg 2");

	multiplyPoints(pointArgs);

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
