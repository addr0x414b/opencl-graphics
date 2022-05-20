#include "clg.hpp"

#include <fstream>
#include <iostream>
#include <unistd.h>


clg::clg(int sWidth, int sHeight) {

	screenWidth = sWidth;
	screenHeight = sHeight;

	platform = cl::Platform::get(&err);
	checkError(err, "Platform creation");

	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
	if (devices.size() == 0) {
		err = -1;
		checkError(err, "Getting devices. No devices found");
	}

	device = devices.front();

	cl::Context c(device, NULL, NULL, NULL, &err);
	checkError(err, "Context creation");
	context = c;

	std::ifstream pipelineFile("../src/opencl/gPipeline.cl");
	std::string pipelineSrc(std::istreambuf_iterator<char>(pipelineFile),
			(std::istreambuf_iterator<char>()));

	char buffer[100];
	getcwd(buffer, sizeof(buffer));
	std::cout << buffer << std::endl;

	cl::Program p(context, pipelineSrc.c_str(), CL_TRUE, &err);
	checkError(err, "Pipeline program creation");
	pipelineProgram = p;

	cl::CommandQueue q(context, device, 0, &err);
	checkError(err, "Queue creation");
	queue = q;
}

void clg::checkError(cl_int err, std::string location) {
	if (err != 0) {
		throw std::runtime_error("Error caught at " + location + ": " +
				std::to_string(err));
	}
}

/* Render triangles in a wireframe mode with dots at each point
 * @param vertices: array of the vertices
 * @param attrCount: number of items combined for each attribute
 * @param tCount: total number in vertices
 * @params lR, lG, lB: line red, green, blue values 0-255,
 * @param lThickness: line thickness
 * @params dR, dG, dB: dot red, green, blue values 0-255,
 * @param dThickness: dot thickness,
 * @params scaleMat, rotMat, transMat, projMat: vertex matrices,
 * @param screenBuffer: array to output pixels */
void clg::drawWireframeDots(
		float* vertices, int attrCount, int tCount,
		int lR, int lG, int lB, int lThickness,
		int dR, int dG, int dB, int dThickness,
		float* scaleMat, float* rotMat, float* transMat, float* projMat,
		int* screenBuffer) {

	cl::EnqueueArgs args(queue, cl::NDRange(tCount));

	cl::Kernel drawWireframeDotsKernel(pipelineProgram, "drawWireframeDots", &err);
	checkError(err, "DrawWireframeDotsKernel creation");
	cl::KernelFunctor<> drawWireframeDots(drawWireframeDotsKernel);

	cl::Buffer inputBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			tCount*sizeof(float), vertices, &err);
	checkError(err, "InputBuf creation");
	err = drawWireframeDotsKernel.setArg(0, sizeof(cl_mem), &inputBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 0 (input)");

	cl::Buffer outputBuf(context, CL_MEM_READ_ONLY,
			tCount*sizeof(float), NULL, &err);
	checkError(err, "OutputBuf creation");
	err = drawWireframeDotsKernel.setArg(1, sizeof(cl_mem), &outputBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 1 (output)");

	err = drawWireframeDotsKernel.setArg(2, sizeof(int), &attrCount);
	checkError(err, "DrawWireframeDotsKernel Arg 2 (attrCount)");

	err = drawWireframeDotsKernel.setArg(3, sizeof(int), &tCount);
	checkError(err, "DrawWireframeDotsKernel Arg 3 (tCount)");

	int lineP[] = { lR, lG, lB, lThickness };
	cl::Buffer lineParams(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			4*sizeof(int), lineP, &err);
	checkError(err, "LineParams creation");
	err = drawWireframeDotsKernel.setArg(4, sizeof(cl_mem), &lineParams);
	checkError(err, "DrawWireframeDotsKernel Arg 4 (lineParams)");

	int dotP[] = { dR, dG, dB, dThickness };
	cl::Buffer dotParams(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			4*sizeof(int), dotP, &err);
	checkError(err, "DotParams creation");
	err = drawWireframeDotsKernel.setArg(5, sizeof(cl_mem), &dotParams);
	checkError(err, "DrawWireframeDotsKernel Arg 5 (dotParams)");

	cl::Buffer scaleMatBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			16*sizeof(float), scaleMat, &err);
	checkError(err, "ScaleMatBuf creation");
	err = drawWireframeDotsKernel.setArg(6, sizeof(cl_mem), &scaleMatBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 6 (scaleMat)");

	cl::Buffer rotMatBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			16*sizeof(float), rotMat, &err);
	checkError(err, "RotMatBuf creation");
	err = drawWireframeDotsKernel.setArg(7, sizeof(cl_mem), &rotMatBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 7 (rotMat)");

	cl::Buffer transMatBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			16*sizeof(float), transMat, &err);
	checkError(err, "TransMatBuf creation");
	err = drawWireframeDotsKernel.setArg(8, sizeof(cl_mem), &transMatBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 8 (transMat)");

	cl::Buffer projMatBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			16*sizeof(float), projMat, &err);
	checkError(err, "ProjmatBuf creation");
	err = drawWireframeDotsKernel.setArg(9, sizeof(cl_mem), &projMatBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 9 (projMat)");

	cl::Buffer scaledOutBuf(context, CL_MEM_READ_WRITE,
			tCount*sizeof(float), NULL, &err);
	checkError(err, "ScaledOutBuf creation");
	err = drawWireframeDotsKernel.setArg(10, sizeof(cl_mem), &scaledOutBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 10 (scaledOut)");

	cl::Buffer rotOutBuf(context, CL_MEM_READ_WRITE,
			tCount*sizeof(float), NULL, &err);
	checkError(err, "RotOutBuf creation");
	err = drawWireframeDotsKernel.setArg(11, sizeof(cl_mem), &rotOutBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 11 (rotOut)");

	cl::Buffer transOutBuf(context, CL_MEM_READ_WRITE,
			tCount*sizeof(float), NULL, &err);
	checkError(err, "TransOutBuf creation");
	err = drawWireframeDotsKernel.setArg(12, sizeof(cl_mem), &transOutBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 12 (transOut)");

	cl::Buffer screenBuf(context, CL_MEM_WRITE_ONLY,
			screenWidth*screenHeight*sizeof(int));
	checkError(err, "ScreenBuf creation");
	err = drawWireframeDotsKernel.setArg(13, sizeof(cl_mem), &screenBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 13 (screen)");

	err = drawWireframeDotsKernel.setArg(14, sizeof(int), &screenWidth);
	checkError(err, "DrawWireframeDotsKernel Arg 14 (screenWidth)");

	err = drawWireframeDotsKernel.setArg(15, sizeof(int), &screenHeight);
	checkError(err, "DrawWireframeDotsKernel Arg 15 (screenHeight)");

	drawWireframeDots(args);
	queue.enqueueReadBuffer(screenBuf, CL_TRUE, 0,
			screenWidth*screenHeight*sizeof(int), screenBuffer);

	/*cl::Buffer vertexScaleBuffer(context, CL_MEM_READ_WRITE,
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


	err = submitVerticesKernel.setArg(1, sizeof(cl_mem), &vertexOutBuffer);
	checkError(err, "SubmitVerticesKernel Arg 1");

	err = submitVerticesKernel.setArg(2, sizeof(cl_mem), &pixelBuffer);
	checkError(err, "SubmitVerticesKernel Arg 2");

	//std::cout << SCREEN_WIDTH << ", " << SCREEN_HEIGHT << std::endl;
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
			SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(int), screenBuffer);*/
}

/* Set the screen size. Useful when updating the screen resolution
 * @params sWidth, sHeight: new screen width and height */
void clg::setScreenWidthHeight(int sWidth, int sHeight) {
	screenWidth = sWidth;
	screenHeight = sHeight;
}
