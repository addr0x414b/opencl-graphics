#pragma once

#define CL_HPP_TARGET_OPENCL_VERSION 300
#include <CL/opencl.hpp>

#include <vector>

struct vec3 {
	float x, y, z;
	vec3() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	vec3(float xx, float yy, float zz) {
		x = xx;
		y = yy;
		z = zz;
	}
};

/* OpenCL graphics class, contains boilerplate for OpenCL */
class clg {
	private:
		cl::Platform platform;
		cl::Device device;
		cl::Context context;
		cl::Program pipelineProgram;
		cl::CommandQueue queue;
		cl::Buffer screenBuf;
		cl_int err;

		int screenWidth;
		int screenHeight;

		/* Check an OpenCL Error code
		 * @param err: OpenCL output error
		 * @param location: a string specifying where the error occurred */
		void checkError(cl_int err, std::string location);

	public:
		/* Default constructor - create the OpenCL graphics object
		 * @params sWidth, sHeight: screen width and height */
		clg(int sWidth, int sHeight);

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
		void drawWireframeDots(
				float* vertices, int attrCount, int tCount,
				int lR, int lG, int lB,
				int dR, int dG, int dB, int dThickness,
				float* scaleMat, float* rotMat, float* transMat, float* viewMat,
				float* projMat, bool drawDots);

		/* Draw triangles utilizing their normals (MUST BE FACE NORMALS, FLAT)
		 * @param vertices: array of the vertices
		 * @param attrCount: number of items combined for each attribute
		 * @param tCount: total number in vertices
		 * @params lR, lG, lB: line red, green, blue values 0-255
		 * @params scaleMat, rotMat, transMat, projMat: vertex matrices
		 * @params camX, camY, camZ: camera X,Y,Z position */
		void drawTrigs(float* vertices, int attrCount, int tCount,
				int lR, int lG, int lB,
				float* scaleMat, float* rotMat, float* transMat, float* viewMat,
				float* projMat, float camX, float camY, float camZ);

		/* Update the screen contents with whatever is inside the screenBuf
		 * @param screen: array of size screenWidth * screenHeight */
		void updateScreen(int* screen);

		/* Set the screen size. Useful when updating the screen resolution
		 * @params sWidth, sHeight: new screen width and height */
		void setScreenWidthHeight(int sWidth, int sHeight);
};

/* Convert degrees to radians
 * @param n: degrees */
float toRad(float n);

/* Create a projection matrix
 * @param fov: field of view
 * @params zNear, zFar: z near and far clipping planes
 * @param m: size 16 array to output */
void createProjMat(float fov, float zNear, float zFar, float* m);

/* Create a scale matrix
 * @param amt: amount to scale
 * @param m: size 16 array to output */
void createScaleMat(float amt, float* m);

/* Create a rotation matrix
 * @params x, y, z: amount to rotate (in degrees) in the x, y, and z direction
 * @param m: size 16 array to output */
void createRotMat(float x, float y, float z, float* m);

/* Create a transformation matrix
 * @params x, y, z: amount to translate in the x, y, and z direction
 * @param m: size 16 array to ouput */
void createTransMat(float x, float y, float z, float* m);

/* Create a lookat matrix
 * @param pos: camera position
 * @param target: camera look at target
 * @param up: camera up vector
 * @param m: size 16 array to output */
void createLookAtMat(vec3 pos, vec3 target, vec3 up, float* m);

/* Subtract two vector 3s. Performs a - b
 * @params a, b: vec 3s
 * @return: vec3 sum */
vec3 subVec(vec3 a, vec3 b);

/* Perform the dot product between two vector 3s
 * @params a, b: vec 3s
 * @return: float dot product */
float dotVec(vec3 a, vec3 b);

/* Normalize a vector 3
 * @param a: vec 3 input
 * @return: vec3 normalized output */
vec3 normalize(vec3 a);

/* Calculate cross product between two vector 3s
 * @params a, b: vec3 inputs
 * @return: vec3 cross product result */
vec3 crossVec(vec3 a, vec3 b);

/* Multiply a vector 3 by a matrix
 * @param a: vec3 input
 * @param m: size 16 array to multiply by
 * @return: vec3 result */
vec3 multiplyVec(vec3 a, float* m);
