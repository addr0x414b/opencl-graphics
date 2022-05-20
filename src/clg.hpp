#pragma once

#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <CL/opencl.hpp>

#include <vector>

class clg {
	private:
		cl::Platform platform;
		cl::Device device;
		cl::Context context;
		cl::Program pipelineProgram;
		cl::CommandQueue queue;
		cl_int err;

		int screenWidth;
		int screenHeight;

		void checkError(cl_int err, std::string location);

	public:
		/* Defaul constructor - create the OpenCL graphics object
		 * @params sWidth, sHeight: screen width and height */
		clg(int sWidth, int sHeight);

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
		void drawWireframeDots(
				float* vertices, int attrCount, int tCount,
				int lR, int lG, int lB, int lThickness,
				int dR, int dG, int dB, int dThickness,
				float* scaleMat, float* rotMat, float* transMat, float* projMat,
				int* screenBuffer);

		/* Set the screen size. Useful when updating the screen resolution
		 * @params sWidth, sHeight: new screen width and height */
		void setScreenWidthHeight(int sWidth, int sHeight);
};
