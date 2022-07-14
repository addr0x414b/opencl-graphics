#include "clg.hpp"

#include <CL/cl.h>
#include <fstream>
#include <iostream>
#include <math.h>

/* Default constructor - create the OpenCL graphics object
 * @params sWidth, sHeight: screen width and height */
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

	/*int num;
	device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &num);
	std::cout << num << std::endl;*/

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

/* Check an OpenCL Error code
 * @param err: OpenCL output error
 * @param location: a string specifying where the error occurred */
void clg::checkError(cl_int err, std::string location) {
	if (err != 0) {
		throw std::runtime_error("Error caught at " + location + ": " +
				std::to_string(err));
	}
}

/* Draw triangles to screen buffer in a wireframe mode with or without
 * dots at each point
 * @param vertices: array of the vertices
 * @param attrCount: number of items combined for each attribute
 * @param tCount: total number in vertices
 * @params lR, lG, lB: line red, green, blue values 0-255,
 * @params dR, dG, dB: dot red, green, blue values 0-255,
 * @param dThickness: dot thickness,
 * @params scaleMat, rotMat, transMat, projMat: vertex matrices
 * @param drawDots: whether or not you want dots to be drawn at each point*/
void clg::drawWireframeDots(
		float* vertices, int attrCount, int tCount,
		int lR, int lG, int lB,
		int dR, int dG, int dB, int dThickness,
		float* scaleMat, float* rotMat, float* transMat, float* viewMat,
		float* projMat, bool drawDots) {

	int padder = 50;
	int gSize = 32;
	int lSize = 32;

	int tester = tCount;
	tester += padder;
	if (tester > 32) {
		if (tester % 32 != 0) {
			while (tester % 32 != 0) {
				tester += 1;
				if (tester % 32 == 0) {
					for (int i = 256; i > 32; i -= 32) {
						if (tester % i == 0) {
							gSize = tester;
							lSize = i;
							break;
						}
					}
				}
			}
		} else {
			for (int i = 256; i > 32; i -= 32) {
				if (tester % i == 0) {
					gSize = tester;
					lSize = i;
					break;
				}
			}
		}
	}

	//cl::EnqueueArgs args(queue, cl::NDRange(gSize), cl::NDRange(lSize));
	cl::EnqueueArgs args(queue, cl::NDRange(tCount*2));
	//std::cout << tCount << ", " << gSize << ", " << lSize << std::endl;
	//std::cout << tCount << std::endl;

	//cl::EnqueueArgs args(queue, cl::NDRange(696969696));

	cl::Kernel drawWireframeDotsKernel(pipelineProgram, "drawWireframeDots", &err);
	checkError(err, "DrawWireframeDotsKernel creation");
	cl::KernelFunctor<> drawWireframeDots(drawWireframeDotsKernel);

	cl::Buffer inputBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			(tCount)*sizeof(float), vertices, &err);
	checkError(err, "InputBuf creation");
	err = drawWireframeDotsKernel.setArg(0, sizeof(cl_mem), &inputBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 0 (input)");

	cl::Buffer outputBuf(context, CL_MEM_READ_ONLY,
			(tCount*2)*sizeof(float), NULL, &err);
	checkError(err, "OutputBuf creation");
	err = drawWireframeDotsKernel.setArg(1, sizeof(cl_mem), &outputBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 1 (output)");

	err = drawWireframeDotsKernel.setArg(2, sizeof(int), &attrCount);
	checkError(err, "DrawWireframeDotsKernel Arg 2 (attrCount)");

	cl::Buffer tCountBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(int), &tCount, &err);
	err = drawWireframeDotsKernel.setArg(3, sizeof(cl_mem), &tCountBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 3 (tCount)");

	int lineP[] = { lR, lG, lB};
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

	cl::Buffer viewMatBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			16*sizeof(float), viewMat, &err);
	checkError(err, "ViewMatBuf creation");
	err = drawWireframeDotsKernel.setArg(9, sizeof(cl_mem), &viewMatBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 9 (viewMat)");

	cl::Buffer projMatBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			16*sizeof(float), projMat, &err);
	checkError(err, "ProjmatBuf creation");
	err = drawWireframeDotsKernel.setArg(10, sizeof(cl_mem), &projMatBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 10 (projMat)");

	cl::Buffer scaledOutBuf(context, CL_MEM_READ_WRITE,
			(tCount+padder)*sizeof(float), NULL, &err);
	checkError(err, "ScaledOutBuf creation");
	err = drawWireframeDotsKernel.setArg(11, sizeof(cl_mem), &scaledOutBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 11 (scaledOut)");

	cl::Buffer rotOutBuf(context, CL_MEM_READ_WRITE,
			(tCount+padder)*sizeof(float), NULL, &err);
	checkError(err, "RotOutBuf creation");
	err = drawWireframeDotsKernel.setArg(12, sizeof(cl_mem), &rotOutBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 12 (rotOut)");

	cl::Buffer transOutBuf(context, CL_MEM_READ_WRITE,
			(tCount+padder)*sizeof(float), NULL, &err);
	checkError(err, "TransOutBuf creation");
	err = drawWireframeDotsKernel.setArg(13, sizeof(cl_mem), &transOutBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 13 (transOut)");

	cl::Buffer viewOutBuf(context, CL_MEM_READ_WRITE,
			(tCount+padder)*sizeof(float), NULL, &err);
	checkError(err, "ViewOutBuf creation");
	err = drawWireframeDotsKernel.setArg(14, sizeof(cl_mem), &viewOutBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 14 (viewOut)");

	cl::Buffer zClipOutBuf(context, CL_MEM_READ_WRITE,
			(tCount)*sizeof(float), NULL, &err);
	checkError(err, "ZClipOutBuf creation");
	err = drawWireframeDotsKernel.setArg(15, sizeof(cl_mem), &zClipOutBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 15 (zClipOut)");

	err = drawWireframeDotsKernel.setArg(16, sizeof(cl_mem), &screenBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 16 (screen)");

	err = drawWireframeDotsKernel.setArg(17, sizeof(int), &screenWidth);
	checkError(err, "DrawWireframeDotsKernel Arg 17 (screenWidth)");

	err = drawWireframeDotsKernel.setArg(18, sizeof(int), &screenHeight);
	checkError(err, "DrawWireframeDotsKernel Arg 18 (screenHeight)");

	int dots = 0;

	if (drawDots == true) {
		dots = 1;
	}

	err = drawWireframeDotsKernel.setArg(19, sizeof(int), &dots);
	checkError(err, "DrawWireframeDotsKernel Arg 19 (dots)");

	cl::Buffer clippedOrigBuf(context, CL_MEM_READ_WRITE,
			(tCount)*sizeof(float), NULL, &err);
	checkError(err, "ClippedOrigBuf creation");
	err = drawWireframeDotsKernel.setArg(20, sizeof(cl_mem), &clippedOrigBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 20 (clippedOrig)");

	cl::Buffer combinedBuf(context, CL_MEM_READ_WRITE,
			(tCount*2)*sizeof(float), NULL, &err);
	checkError(err, "CombinedBuf creation");
	err = drawWireframeDotsKernel.setArg(21, sizeof(cl_mem), &combinedBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 21 (combined)");

	drawWireframeDots(args);

}

/* Draw triangles to screen buffer in a wireframe mode with or without
 * dots at each point
 * @param vertices: array of the vertices
 * @param attrCount: number of items combined for each attribute
 * @param tCount: total number in vertices
 * @params lR, lG, lB: line red, green, blue values 0-255,
 * @params dR, dG, dB: dot red, green, blue values 0-255,
 * @param dThickness: dot thickness,
 * @params scaleMat, rotMat, transMat, projMat: vertex matrices
 * @param drawDots: whether or not you want dots to be drawn at each point*/
/*void clg::drawWireframeDots(float* vertices, int attrCount, int tCount) {
	int gSize = 32;
	int lSize = 32;

	int tester = tCount;
	if (tester > 32) {
		while (tester % 32 != 0) {
			tester += 1;
			if (tester % 32 == 0) {
				for (int i = 256; i > 32; i -= 32) {
					if (tester % i == 0) {
						gSize = tester;
						lSize = i;
					}
				}
			}
		}
	}

	//std::cout << tester << ", " << gSize << ", " << lSize << ", " << tCount << std::endl;

	cl::EnqueueArgs args(queue, cl::NDRange(gSize), cl::NDRange(lSize));

	cl::Kernel drawWireframeDotsKernel(pipelineProgram, "drawWireframeDots", &err);
	checkError(err, "DrawWireframeDotsKernel creation");
	cl::KernelFunctor<> drawWireframeDots(drawWireframeDotsKernel);

	cl::Buffer inputBuf(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			tCount*sizeof(float), vertices, &err);
	checkError(err, "InputBuf creation");
	err = drawWireframeDotsKernel.setArg(0, sizeof(cl_mem), &inputBuf);
	checkError(err, "DrawWireframeDotsKernel Arg 0 (input)");

	err = drawWireframeDotsKernel.setArg(1, sizeof(int), &attrCount);
	checkError(err, "DrawWireframeDotsKernel Arg 1 (attrCount)");

	err = drawWireframeDotsKernel.setArg(2, sizeof(int), &tCount);
	checkError(err, "DrawWireframeDotsKernel Arg 2 (tCount)");

	drawWireframeDots(args);

}*/


/* Set the screen size. Useful when updating the screen resolution
 * @params sWidth, sHeight: new screen width and height */
void clg::setScreenWidthHeight(int sWidth, int sHeight) {
	screenWidth = sWidth;
	screenHeight = sHeight;
}

/* Update the screen contents with whatever is inside the screenBuf
 * @param screen: array of size screenWidth * screenHeight */
void clg::updateScreen(int* screen) {
	if (screen != NULL) {
		queue.enqueueReadBuffer(screenBuf, CL_TRUE, 0,
				screenWidth*screenHeight*sizeof(int), screen);
	}
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

/* Create a lookat matrix
 * @param pos: camera position
 * @param target: camera look at target
 * @param up: camera up vector
 * @param m: size 16 array to output */
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

/* Subtract two vector 3s. Performs a - b
 * @params a, b: vec 3s */
vec3 subVec(vec3 a, vec3 b) {
	vec3 ans;
	ans.x = a.x - b.x;
	ans.y = a.y - b.y;
	ans.z = a.z - b.z;
	return ans;
}

/* Perform the dot product between two vector 3s
 * @params a, b: vec 3s
 * @return: float dot product */
float dotVec(vec3 a, vec3 b) {
	return (a.x*b.x) + (a.y*b.y) + (a.z*b.z);
}

/* Normalize a vector 3
 * @param a: vec 3 input
 * @return: vec3 normalized output */
vec3 normalize(vec3 a) {
	vec3 ans;
	float l = sqrtf(dotVec(a, a));
	ans.x = a.x / l;
	ans.y = a.y / l;
	ans.z = a.z / l;
	return ans;
}

/* Calculate cross product between two vector 3s
 * @params a, b: vec3 inputs
 * @return: vec3 cross product result */
vec3 crossVec(vec3 a, vec3 b) {
	vec3 ans;
	ans.x = (a.y * b.z) - (a.z * b.y);
	ans.y = (a.z * b.x) - (a.x * b.z);
	ans.z = (a.x * b.y) - (a.y * b.x);
	return ans;
}

/* Multiply a vector 3 by a matrix
 * @param a: vec3 input
 * @param m: size 16 array to multiply by
 * @return: vec3 result */
vec3 multiplyVec(vec3 a, float* m) {
	vec3 ans;

	ans.x = (a.x * m[0]) + (a.y * m[1]) + (a.z * m[2]) + m[3];
	ans.y = (a.x * m[4]) + (a.y * m[5]) + (a.z * m[6]) + m[7];
	ans.z = (a.x * m[8]) + (a.y * m[9]) + (a.z * m[10]) + m[11];
	float w = (a.x * m[12]) + (a.y * m[13]) + (a.z * m[14]) + m[15];

	if (w != 0.0f) {
		ans.x /= w;
		ans.y /= w;
		ans.z /= w;
	}

	return ans;
}
