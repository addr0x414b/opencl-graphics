#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <CL/opencl.hpp>
#include <SDL2/SDL.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>

int SCREEN_WIDTH = 640;
int SCREEN_HEIGHT = 480;
float FOV = 60.0f;


void checkError(cl_int err, std::string location) {
	if (err != 0) {
		throw std::runtime_error("Error caught at " + location + ": " +
				std::to_string(err));
	}
}

float toRad(float n) {
	return n * (3.141592f / 180.0f);
}

void createProjMat(float fov, float zNear, float zFar, float* m) {
	float fovRad = toRad(fov/2.0f);

	m[0] = (1.0f / tanf(fovRad));
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = (1.0f / tanf(fovRad));
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = ((-2.0f * zNear) / (zFar - zNear)) - 1.0f;
	m[11] = (-zNear * zFar) / (zFar - zNear);

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = -1.0f;
	m[15] = 0.0f;
}

void createScaleMat(float amt, float* m) {

	m[0] = amt;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = amt;
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = amt;
	m[11] = 0.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
}

void createRotMat(float x, float y, float z, float* m) {

	float xRot = toRad(x);
	float yRot = toRad(y);
	float zRot = toRad(z);

	m[0] = cosf(yRot) * cosf(zRot);
	m[1] = cosf(yRot) * sinf(zRot);
	m[2] = -sinf(yRot);
	m[3] = 0.0f;

	m[4] = (sinf(xRot) * sinf(yRot) * cosf(zRot)) -
			(cosf(xRot) * sinf(zRot));
	m[5] = (sinf(xRot) * sinf(yRot) * sinf(zRot)) +
			(cosf(xRot) * cosf(zRot));
	m[6] = sinf(xRot) * cosf(yRot);
	m[7] = 0.0f;

	m[8] = (cosf(xRot) * sinf(yRot) * cosf(zRot)) +
			(sinf(xRot) * sinf(zRot));
	m[9] = (cosf(xRot) * sinf(yRot) * sinf(zRot)) -
			(sinf(xRot) * cosf(zRot));
	m[10] = cosf(xRot) * cosf(yRot);
	m[11] = 0.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 0.0f;

}

void createTransMat(float x, float y, float z, float* m) {

	m[0] = 1.0f;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = x;

	m[4] = 0.0f;
	m[5] = 1.0f;
	m[6] = 0.0f;
	m[7] = y;

	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = 1.0f;
	m[11] = z;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = 0.0f;
	m[15] = 1.0f;
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

	float fovRad = (FOV/2.0f) * (3.141592f / 180.0f);
	float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

	/*float projMat[4][4] = {0.0f};
	  projMat[0][0] = (1.0f / tanf(fovRad)) / aspect;
	  projMat[1][1] = (1.0f / tanf(fovRad));
	  projMat[2][2] = ((-2.0f * 0.1f) / (1000.0f - 0.1f)) - 1.0f;
	  projMat[2][3] = -1.0f;
	  projMat[3][2] = (-0.1f * 1000.0f) / (1000.0f - 0.1f);
	  float* projMatPtr = projMat[0];

	  float scaleAmt = 1.f;
	  float scaleMat[4][4] = {0.0f};
	  scaleMat[0][0] = scaleAmt;
	  scaleMat[1][1] = scaleAmt;
	  scaleMat[2][2] = scaleAmt;
	  scaleMat[3][3] = 1.0f;
	  float* scaleMatPtr = scaleMat[0];

	  float xRot = toRad(0.0f);
	  float yRot = toRad(50.0f);
	  float zRot = toRad(0.0f);
	  float rotMat[4][4] = {0.0f};
	  rotMat[0][0] = cosf(yRot) * cosf(zRot);
	  rotMat[0][1] = cosf(yRot) * sinf(zRot);
	  rotMat[0][2] = -sinf(yRot);

	  rotMat[1][0] = (sinf(xRot) * sinf(yRot) * cosf(zRot)) -
	  (cosf(xRot) * sinf(zRot));
	  rotMat[1][1] = (sinf(xRot) * sinf(yRot) * sinf(zRot)) +
	  (cosf(xRot) * cosf(zRot));
	  rotMat[1][2] = sinf(xRot) * cosf(yRot);

	  rotMat[2][0] = (cosf(xRot) * sinf(yRot) * cosf(zRot)) +
	  (sinf(xRot) * sinf(zRot));
	  rotMat[2][1] = (cosf(xRot) * sinf(yRot) * sinf(zRot)) -
	  (sinf(xRot) * cosf(zRot));
	  rotMat[2][2] = cosf(xRot) * cosf(yRot);

	  rotMat[3][3] = 1.0f;
	  rotMat[0][0] = cosf(zRot);
	  rotMat[0][1] = -sinf(zRot);
	  rotMat[1][0] = sinf(zRot);
	  rotMat[1][1] = cosf(zRot);*/

	/*rotMat[0][0] = cosf(zRot);
	  rotMat[0][2] = -sinf(zRot);
	  rotMat[1][1] = 1.0f;
	  rotMat[2][0] = sinf(zRot);
	  rotMat[2][2] = cosf(zRot);
	  rotMat[3][3] = 1.0f;
	  float* rotMatPtr = rotMat[0];

	  float xAmt = 0.0f;
	  float yAmt = 0.0f;
	  float zAmt = -1.f;
	  float transMat[4][4] = {0.0f};
	  transMat[0][0] = 1.0f;
	  transMat[0][3] = xAmt;
	  transMat[1][1] = 1.0f;
	  transMat[1][3] = yAmt;
	  transMat[2][2] = 1.0f;
	  transMat[2][3] = zAmt;
	  transMat[3][3] = 1.0f;
	  float* transMatPtr = transMat[0];

	  float vertices[] = {
	  -5.0f, -5.0f, 0.0f,
	  5.0f, -5.0f, 0.0f,
	  -5.0f, 5.0f, 0.0f,
	  5.0f, 5.0f, 0.0f
	  };
	//4.69846, -5, 0.7101
	float t[] = {-4.69846f, -5.0f, -2.7101f};
	std::cout << "ORIGINAL POINT: " << vertices[0] << ", " << vertices[1] <<
	", " << vertices[2] << std::endl;
	multiply(t, projMat);


	int pointCount = sizeof(vertices) / sizeof(float);

	cl::EnqueueArgs vertexArgs(queue, cl::NDRange(pointCount));

	cl::Buffer vertexBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
	pointCount*sizeof(float), vertices, &err);
	checkError(err, "VertexBuffer");

	cl::Buffer vertexOutBuffer(context, CL_MEM_READ_WRITE,
	pointCount*sizeof(float), NULL, &err);
	checkError(err, "VertexOutBuffer");

	cl::Buffer vertexScaleBuffer(context, CL_MEM_READ_WRITE,
	pointCount*sizeof(float), NULL, &err);
	checkError(err, "VertexScaleBuffer");

	cl::Buffer vertexRotateBuffer(context, CL_MEM_READ_WRITE,
	pointCount*sizeof(float), NULL, &err);
	checkError(err, "VertexRotateBuffer");

	cl::Buffer vertexTranslateBuffer(context, CL_MEM_READ_WRITE,
	pointCount*sizeof(float), NULL, &err);
	checkError(err, "VertexTranslateBuffer");

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

	cl::Buffer projMatBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			16*sizeof(float), projMatPtr, &err);
	checkError(err, "ProjMatBuffer");

	err = submitVerticesKernel.setArg(6, sizeof(cl_mem), &projMatBuffer);
	checkError(err, "SubmitVerticesKernel Arg 6");

	cl::Buffer scaleMatBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			16*sizeof(float), scaleMatPtr, &err);
	checkError(err, "ScaleMatBuffer");

	err = submitVerticesKernel.setArg(7, sizeof(cl_mem), &scaleMatBuffer);
	checkError(err, "SubmitVerticesKernel Arg 7");

	cl::Buffer rotMatBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			16*sizeof(float), rotMatPtr, &err);
	checkError(err, "RotMatBuffer");

	err = submitVerticesKernel.setArg(8, sizeof(cl_mem), &rotMatBuffer);
	checkError(err, "SubmitVerticesKernel Arg 8");

	cl::Buffer transMatBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			16*sizeof(float), transMatPtr, &err);
	checkError(err, "TransMatBuffer");

	err = submitVerticesKernel.setArg(9, sizeof(cl_mem), &transMatBuffer);
	checkError(err, "SubmitVerticesKernel Arg 9");

	err = submitVerticesKernel.setArg(10, sizeof(cl_mem), &vertexScaleBuffer);
	checkError(err, "SubmitVerticesKernel Arg 10");

	err = submitVerticesKernel.setArg(11, sizeof(cl_mem), &vertexRotateBuffer);
	checkError(err, "SubmitVerticesKernel Arg 11");

	err = submitVerticesKernel.setArg(12, sizeof(cl_mem), &vertexTranslateBuffer);
	checkError(err, "SubmitVerticesKernel Arg 12");

	submitVertices(vertexArgs);

	int screenBuffer[SCREEN_WIDTH*SCREEN_HEIGHT];
	queue.enqueueReadBuffer(pixelBuffer, CL_TRUE, 0,
			SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(int), screenBuffer);*/

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


	SDL_Event event;
	int running = 1;

	float rot = 0.0f;
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
		}
		/*float projMat[4][4] = {0.0f};
		float fovRad = toRad(FOV/2.0f);
		projMat[0][0] = (1.0f / tanf(fovRad));
		projMat[1][1] = 1.0f / tanf(fovRad);
		projMat[2][2] = ((-2.0f * 0.1f) / (1000.0f - 0.1f)) - 1.0f;
		projMat[2][3] = (-0.1f * 1000.0f) / (1000.0f - 0.1f);
		projMat[3][2] = -1.0f;
		float* projMatPtr = projMat[0];*/

		float projMat[16];
		createProjMat(60.0f, 0.1f, 1000.0f, projMat);

		float scaleMat[16];
		createScaleMat(5.f, scaleMat);

		/*float scaleAmt = 5.f;
		float scaleMat[4][4] = {0.0f};
		scaleMat[0][0] = scaleAmt;
		scaleMat[1][1] = scaleAmt;
		scaleMat[2][2] = scaleAmt;
		scaleMat[3][3] = 1.0f;
		float* scaleMatPtr = scaleMat[0];*/

		float rotMat[16];
		createRotMat(0.0f, rot, rot, rotMat);
		rot += 0.5f;

		/*float xRot = toRad(0.0f);
		float yRot = toRad(y);
		float zRot = toRad(y);
		y += 0.5f;
		float rotMat[4][4] = {0.0f};
		rotMat[0][0] = cosf(yRot) * cosf(zRot);
		rotMat[0][1] = cosf(yRot) * sinf(zRot);
		rotMat[0][2] = -sinf(yRot);

		rotMat[1][0] = (sinf(xRot) * sinf(yRot) * cosf(zRot)) -
			(cosf(xRot) * sinf(zRot));
		rotMat[1][1] = (sinf(xRot) * sinf(yRot) * sinf(zRot)) +
			(cosf(xRot) * cosf(zRot));
		rotMat[1][2] = sinf(xRot) * cosf(yRot);

		rotMat[2][0] = (cosf(xRot) * sinf(yRot) * cosf(zRot)) +
			(sinf(xRot) * sinf(zRot));
		rotMat[2][1] = (cosf(xRot) * sinf(yRot) * sinf(zRot)) -
			(sinf(xRot) * cosf(zRot));
		rotMat[2][2] = cosf(xRot) * cosf(yRot);
		float* rotMatPtr = rotMat[0];*/

		float transMat[16];
		createTransMat(0.0f, 0.0f, -15.0f, transMat);
		/*float xAmt = 0.0f;
		float yAmt = 0.0f;
		float zAmt = -15.0f;
		float transMat[4][4] = {0.0f};
		transMat[0][0] = 1.0f;
		transMat[0][3] = xAmt;
		transMat[1][1] = 1.0f;
		transMat[1][3] = yAmt;
		transMat[2][2] = 1.0f;
		transMat[2][3] = zAmt;
		transMat[3][3] = 1.0f;
		float* transMatPtr = transMat[0];*/

		float size = 1.0f;
		float amt = 0.f;
		float vertices[] = {
			-size, size, size-amt,
			size, -size, size-amt,
			-size, -size, size-amt,
			size, size, size-amt,
			-size, size, -size-amt,
			-size, -size, -size-amt,
			size, size, -size-amt,
			size, -size, -size-amt
		};

		int pointCount = sizeof(vertices) / sizeof(float);

		cl::EnqueueArgs vertexArgs(queue, cl::NDRange(pointCount));

		cl::Buffer vertexBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				pointCount*sizeof(float), vertices, &err);
		checkError(err, "VertexBuffer");

		cl::Buffer vertexOutBuffer(context, CL_MEM_READ_WRITE,
				pointCount*sizeof(float), NULL, &err);
		checkError(err, "VertexOutBuffer");

		cl::Buffer vertexScaleBuffer(context, CL_MEM_READ_WRITE,
				pointCount*sizeof(float), NULL, &err);
		checkError(err, "VertexScaleBuffer");

		cl::Buffer vertexRotateBuffer(context, CL_MEM_READ_WRITE,
				pointCount*sizeof(float), NULL, &err);
		checkError(err, "VertexRotateBuffer");

		cl::Buffer vertexTranslateBuffer(context, CL_MEM_READ_WRITE,
				pointCount*sizeof(float), NULL, &err);
		checkError(err, "VertexTranslateBuffer");

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

		cl::Buffer projMatBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				16*sizeof(float), projMat, &err);
		checkError(err, "ProjMatBuffer");

		err = submitVerticesKernel.setArg(6, sizeof(cl_mem), &projMatBuffer);
		checkError(err, "SubmitVerticesKernel Arg 6");

		cl::Buffer scaleMatBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				16*sizeof(float), scaleMat, &err);
		checkError(err, "ScaleMatBuffer");

		err = submitVerticesKernel.setArg(7, sizeof(cl_mem), &scaleMatBuffer);
		checkError(err, "SubmitVerticesKernel Arg 7");

		cl::Buffer rotMatBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				16*sizeof(float), rotMat, &err);
		checkError(err, "RotMatBuffer");

		err = submitVerticesKernel.setArg(8, sizeof(cl_mem), &rotMatBuffer);
		checkError(err, "SubmitVerticesKernel Arg 8");

		cl::Buffer transMatBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				16*sizeof(float), transMat, &err);
		checkError(err, "TransMatBuffer");

		err = submitVerticesKernel.setArg(9, sizeof(cl_mem), &transMatBuffer);
		checkError(err, "SubmitVerticesKernel Arg 9");

		err = submitVerticesKernel.setArg(10, sizeof(cl_mem), &vertexScaleBuffer);
		checkError(err, "SubmitVerticesKernel Arg 10");

		err = submitVerticesKernel.setArg(11, sizeof(cl_mem), &vertexRotateBuffer);
		checkError(err, "SubmitVerticesKernel Arg 11");

		err = submitVerticesKernel.setArg(12, sizeof(cl_mem), &vertexTranslateBuffer);
		checkError(err, "SubmitVerticesKernel Arg 12");

		submitVertices(vertexArgs);

		int screenBuffer[SCREEN_WIDTH*SCREEN_HEIGHT];
		queue.enqueueReadBuffer(pixelBuffer, CL_TRUE, 0,
				SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(int), screenBuffer);

		SDL_UpdateTexture(screen, NULL, screenBuffer, SCREEN_WIDTH*sizeof(int));
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, screen, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	SDL_Quit();
}
