#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <CL/opencl.hpp>
#include <SDL2/SDL.h>

#include <iostream>
#include <vector>
#include <fstream>

int main() {

	cl::Platform platform = cl::Platform::get();
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
	cl::Device device = devices.front();

	auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
	auto name = device.getInfo<CL_DEVICE_NAME>();
	auto version = device.getInfo<CL_DEVICE_VERSION>();

	std::cout << vendor << ", " << name << ", " << version << std::endl;

	std::ifstream file("../test.cl");

	std::string fileSrc(std::istreambuf_iterator<char>(file),
			(std::istreambuf_iterator<char>()));

	cl::Context context(device);
	cl_int error;
	cl::Program program(context, fileSrc.c_str(), CL_TRUE, &error);
	std::cout << "Program Error Code: " << error << std::endl;

	cl::CommandQueue queue(context, device, 0, &error);
	std::cout << "Queue Error Code: " << error << std::endl;

	cl::EnqueueArgs args(queue, cl::NDRange(2));

	cl::Kernel kernel(program, "test", &error);
	cl::KernelFunctor<> test(kernel);
	std::cout << "Kernel Error Code: " << error << std::endl;


	int point[] = {
		320, 240
	};

	int out[2];
	cl::Buffer inBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			2*sizeof(point), point, &error);
	std::cout << "Error Code: " << error << std::endl;
	error = kernel.setArg(0, sizeof(cl_mem), &inBuf);
	std::cout << "Error Code: " << error << std::endl;

	cl::Buffer outBuf(context, CL_MEM_WRITE_ONLY, sizeof(out));
	error = kernel.setArg(1, outBuf);
	std::cout << "Error Code: " << error << std::endl;
	test(args);

	queue.enqueueReadBuffer(outBuf, CL_TRUE, 0, sizeof(out), out);

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window;
	window = SDL_CreateWindow(
			"OpenCL Graphics",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			640,
			480,
			0);

	if (window == NULL) {
		std::cout << "Failed to create window" << std::endl;
		return 1;
	}

	SDL_Event event;
	int running = 1;

	SDL_Renderer* renderer;
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
			SDL_RENDERER_PRESENTVSYNC);


	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderDrawPoint(renderer, out[0], out[1]);
		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
}
