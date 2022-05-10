#include <SDL2/SDL_blendmode.h>
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

	float point[] = {
		0.25f, 0.75f, 0.75f, 0.75f, 0.5f, 0.25f
	};

	cl::EnqueueArgs args(queue, cl::NDRange(sizeof(point) / sizeof(float)));

	cl::Kernel kernel(program, "test", &error);
	cl::KernelFunctor<> test(kernel);
	std::cout << "Kernel Error Code: " << error << std::endl;



	int out[6];
	cl::Buffer inBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			6*sizeof(point), point, &error);
	std::cout << "Error Code: " << error << std::endl;
	error = kernel.setArg(0, sizeof(cl_mem), &inBuf);
	std::cout << "Error Code: " << error << std::endl;

	cl::Buffer outBuf(context, CL_MEM_WRITE_ONLY, sizeof(out));
	error = kernel.setArg(1, outBuf);
	std::cout << "Error Code: " << error << std::endl;
	test(args);

	queue.enqueueReadBuffer(outBuf, CL_TRUE, 0, sizeof(out), out);

	/*std::cout << out[0] << ", " << out[1] << std::endl;
	std::cout << out[2] << ", " << out[3] << std::endl;
	std::cout << out[4] << ", " << out[5] << std::endl;*/

	cl::Kernel kernel2(program, "test2", &error);
	cl::KernelFunctor<> test2(kernel2);
	std::cout << "Kernel Error Code: " << error << std::endl;

	cl::Buffer in2Buf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			6*sizeof(out), out, &error);
	std::cout << "Error Code: " << error << std::endl;
	error = kernel2.setArg(0, sizeof(cl_mem), &in2Buf);
	std::cout << "Error Code: " << error << std::endl;

	//uint32_t* textureBuffer = new uint32_t[640*480];
	uint32_t textureBuffer[640*480];

	cl::Buffer out2Buf(context, CL_MEM_WRITE_ONLY, sizeof(textureBuffer));
	error = kernel2.setArg(1, out2Buf);
	std::cout << "Error Code: " << error << std::endl;

	test2(args);

	queue.enqueueReadBuffer(out2Buf, CL_TRUE, 0, sizeof(textureBuffer), textureBuffer);

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

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, 640, 480);

	/*textureBuffer[(out[1]*640) + out[0]] = 0xFF000000 | 0x00FF0000 | 0x0000FF00
		| 0x000000FF;

	textureBuffer[(out[3]*640) + out[2]] = 0xFF000000 | 0x00FF0000 | 0x0000FF00
		| 0x000000FF;

	textureBuffer[(out[5]*640) + out[4]] = 0xFF000000 | 0x00FF0000 | 0x0000FF00
		| 0x000000FF;*/

	SDL_UpdateTexture(texture, NULL, textureBuffer, 640*sizeof(uint32_t));
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		//SDL_RenderDrawPoint(renderer, out[0], out[1]);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}


	SDL_Quit();
	//delete[] textureBuffer;
}
