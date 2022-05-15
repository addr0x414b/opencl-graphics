#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <CL/opencl.hpp>
#include <SDL2/SDL.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>

int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;

void checkError(cl_int err, std::string location) {
	if (err != 0) {
		throw std::runtime_error("Error caught at " + location + ": " +
				std::to_string(err));
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

	std::ifstream pipelineFile("../src/opencl/graphicsPipeline.cl");
	std::string pipelineSrc(std::istreambuf_iterator<char>(pipelineFile),
			(std::istreambuf_iterator<char>()));

	cl_int err;
	cl::Program pipelineProgram(context, pipelineSrc.c_str(), CL_TRUE, &err);
	checkError(err, "PipelineProgram");

	cl::CommandQueue queue(context, device, 0, &err);
	checkError(err, "Queue");

	float vertices[] = {
		-5.5f, -5.5f, -1.f,
		5.5f, -5.5f, -1.f,
		0.0f, 5.5f, -1.f
	};
	int pointCount = sizeof(vertices) / sizeof(float);

	cl::EnqueueArgs vertexArgs(queue, cl::NDRange(pointCount));

	cl::Buffer vertexBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			pointCount*sizeof(float), vertices, &err);
	checkError(err, "VertexBuffer");

	cl::Buffer vertexOutBuffer(context, CL_MEM_READ_WRITE,
			pointCount*sizeof(float), NULL, &err);
	checkError(err, "VertexOutBuffer");

	cl::Buffer pixelBuffer(context, CL_MEM_WRITE_ONLY,
			SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(int));
	checkError(err, "PixelBuffer");

	cl::Kernel submitVerticesKernel(pipelineProgram, "submitVertices", &err);
	checkError(err, "SubmitVerticesKernel");
	cl::KernelFunctor<> submitVertices(submitVerticesKernel);

	err = submitVerticesKernel.setArg(0, sizeof(cl_mem), &vertexBuffer);
	checkError(err, "SubmitVerticesKernel Arg 0");

	err = submitVerticesKernel.setArg(1, sizeof(cl_mem), &vertexOutBuffer);
	checkError(err, "SubmitVerticesKernel Arg 1");

	err = submitVerticesKernel.setArg(2, sizeof(cl_mem), &pixelBuffer);
	checkError(err, "SubmitVerticesKernel Arg 2");

	err = submitVerticesKernel.setArg(3, sizeof(int), &SCREEN_WIDTH);
	checkError(err, "SubmitVerticesKernel Arg 3");

	err = submitVerticesKernel.setArg(4, sizeof(int), &SCREEN_HEIGHT);
	checkError(err, "SubmitVerticesKernel Arg 4");

	err = submitVerticesKernel.setArg(5, sizeof(int), &pointCount);
	checkError(err, "SubmitVerticesKernel Arg 5");

	submitVertices(vertexArgs);

	int screenBuffer[SCREEN_WIDTH*SCREEN_HEIGHT];
	queue.enqueueReadBuffer(pixelBuffer, CL_TRUE, 0,
			SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(int), screenBuffer);

	/*cl::EnqueueArgs args(queue, cl::NDRange(3));

	int nums[] = {2, 2, 5, 4, 9, 9};
	int output[3];

	cl::Buffer numsBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			6*sizeof(int), nums, &err);
	checkError(err, "NumsBuffer");

	cl::Buffer outputBuffer(context, CL_MEM_WRITE_ONLY, 3*sizeof(int), NULL, &err);
	checkError(err, "OutputBuffer");

	cl::Kernel helloWorldKernel(pipelineProgram, "helloWorld", &err);
	checkError(err, "HelloWorldKernel");
	cl::KernelFunctor<> helloWorld(helloWorldKernel);

	err = helloWorldKernel.setArg(0, sizeof(cl_mem), &numsBuffer);
	checkError(err, "HelloWorldKernel Arg 0");

	err = helloWorldKernel.setArg(1, sizeof(cl_mem), &outputBuffer);
	checkError(err, "HelloWorldKernel Arg 1");

	helloWorld(args);

	queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, 3*sizeof(int), output);

	std::cout << output[0] << ", " << output[1] << ", " << output[2] << std::endl;*/

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow(
			"OpenCL Graphics",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			SCREEN_WIDTH,
			SCREEN_HEIGHT,
			0);

	if (window == NULL) {
		throw std::runtime_error("Failed to create window");
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	SDL_Texture* screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetTextureBlendMode(screen, SDL_BLENDMODE_BLEND);

	SDL_UpdateTexture(screen, NULL, screenBuffer, SCREEN_WIDTH*sizeof(int));

	SDL_Event event;
	int running = 1;

	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, screen, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
}
