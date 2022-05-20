#include "clg.hpp"

#include <fstream>
#include <iostream>
#include <math.h>


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

	cl::Program p(context, pipelineSrc.c_str(), CL_TRUE, &err);
	checkError(err, "Pipeline program creation");
	pipelineProgram = p;

	cl::CommandQueue q(context, device, 0, &err);
	checkError(err, "Queue creation");
	queue = q;

	cl::Buffer s(context, CL_MEM_WRITE_ONLY,
			screenWidth*screenHeight*sizeof(int), NULL, &err);
	checkError(err, "ScreenBuf creation");
	screenBuf = s;
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
		float* scaleMat, float* rotMat, float* transMat, float* projMat) {

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

	err = drawWireframeDotsKernel.setArg(13, sizeof(cl_mem), &screenBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 13 (screen)");

	err = drawWireframeDotsKernel.setArg(14, sizeof(int), &screenWidth);
	checkError(err, "DrawWireframeDotsKernel Arg 14 (screenWidth)");

	err = drawWireframeDotsKernel.setArg(15, sizeof(int), &screenHeight);
	checkError(err, "DrawWireframeDotsKernel Arg 15 (screenHeight)");

	drawWireframeDots(args);
}

/* Set the screen size. Useful when updating the screen resolution
 * @params sWidth, sHeight: new screen width and height */
void clg::setScreenWidthHeight(int sWidth, int sHeight) {
	screenWidth = sWidth;
	screenHeight = sHeight;
}

/* Update the screen contents with whatever is inside the screenBuf
 * @param screen: array of size screenWidth * screenHeight */
void clg::updateScreen(int* screen) {
	queue.enqueueReadBuffer(screenBuf, CL_TRUE, 0,
			screenWidth*screenHeight*sizeof(int), screen);
	cl::Buffer s(context, CL_MEM_WRITE_ONLY,
			screenWidth*screenHeight*sizeof(int), NULL, &err);
	checkError(err, "ScreenBuf re-creation");
	screenBuf = s;
}

/* Convert degrees to radians
 * @param n: degrees */
float toRad(float n) {
	return n * (3.141592f / 180.0f);
}

/* Create a projection matrix
 * @param fov: field of view
 * @params zNear, zFar: z near and far clipping planes
 * @param m: size 16 array */
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

/* Create a scale matrix
 * @param amt: amount to scale
 * @param m: size 16 array to output */
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

/* Create a rotation matrix
 * @params x, y, z: amount to rotate (in degrees) in the x, y, and z direction
 * @param m: size 16 array to output */
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

/* Create a transformation matrix
 * @params x, y, z: amount to translate in the x, y, and z direction
 * @param m: size 16 array to ouput */
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
