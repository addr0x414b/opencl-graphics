#include <SDL2/SDL_blendmode.h>
#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <CL/opencl.hpp>
#include <SDL2/SDL.h>

#include <iostream>
#include <vector>
#include <fstream>

int WIDTH = 640;
int HEIGHT = 480;

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
	std::cout << "Vertex Program Error: " << err << std::endl;

	cl::CommandQueue queue(context, device, 0, &err);
	std::cout << "Queue Error: " << err << std::endl;

	/* x, y, z */
	float points[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.0f, 0.5f, -0.5f
	};

	int pointsCount = sizeof(points) / sizeof(float);

	cl::EnqueueArgs pointArgs(queue, cl::NDRange(pointsCount));

	cl::Kernel centerFlipYKernel(vertexProgram, "centerFlipY", &err);
	std::cout << "CenterFlipY Kernel Error: " << err << std::endl;
	cl::KernelFunctor<> centerFlipY(centerFlipYKernel);

	cl::Buffer pointsBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			pointsCount*sizeof(points), points, &err);
	std::cout << "PointsBuffer Error: " << err << std::endl;
	err = centerFlipYKernel.setArg(0, sizeof(cl_mem), &pointsBuffer);
	std::cout << "CenterFlipYKernel Arg 0 Error: " << err << std::endl;

	int pointsOut[pointsCount];
	cl::Buffer pointsOutBuffer(context, CL_MEM_READ_WRITE, sizeof(pointsOut));
	err = centerFlipYKernel.setArg(1, pointsOutBuffer);
	std::cout << "CenterFlipYKernel Arg 1 Error: " << err << std::endl;

	cl::Buffer screenWidthBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(WIDTH), &WIDTH, &err);
	err = centerFlipYKernel.setArg(2, screenWidthBuffer);
	std::cout << "CenterFlipYKernel Arg 2 Error: " << err << std::endl;

	cl::Buffer screenHeightBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(HEIGHT), &HEIGHT, &err);
	err = centerFlipYKernel.setArg(3, screenHeightBuffer);
	std::cout << "CenterFlipYKernel Arg 3 Error: " << err << std::endl;

	centerFlipY(pointArgs);

	cl::Kernel drawPointsKernel(vertexProgram, "drawPoints", &err);
	std::cout << "DrawPointsKernel Error: " << err << std::endl;
	cl::KernelFunctor<> drawPoints(drawPointsKernel);

	err = drawPointsKernel.setArg(0, sizeof(cl_mem), &pointsOutBuffer);
	std::cout << "DrawPointsKernel Arg 0 Error: " << err << std::endl;

	uint32_t textureBuffer[WIDTH*HEIGHT];
	cl::Buffer pixelBuffer(context, CL_MEM_WRITE_ONLY, sizeof(textureBuffer));
	err = drawPointsKernel.setArg(1, pixelBuffer);
	std::cout << "DrawPointsKernel Arg 1 Error: " << err << std::endl;

	err = drawPointsKernel.setArg(2, screenWidthBuffer);
	std::cout << "DrawPointsKernel Arg 2 Error: " << err << std::endl;

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
		std::cout << "Failed to create window" << std::endl;
		return 1;
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
	//delete[] textureBuffer;
}
