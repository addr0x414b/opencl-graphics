__kernel void shadeTrigsKernel(
	__global float* input, __global float* output,
	int attrCount, int tCount,
	__global float* output2, __global float* zClipOut, __global float* clippedOrig,
	__global int* lineParams, __global float* scaleMat, __global float* scaledOut,
	__global float* rotMat, __global float* rotOut,
	__global float* transMat, __global float* transOut,
	__global float* viewMat, __global float* viewOut,
	__global float* projMat,
	__global int* screen, int screenWidth, int screenHeight,
	__global float* camPos, __global float* visibleOut,
	__global float* lightParams, __global int* aLightParams) {

		multiply(input, scaledOut, scaleMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(scaledOut, rotOut, rotMat, attrCount, tCount, 1);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(rotOut, transOut, transMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		visibleTrigs(transOut, visibleOut, camPos[0], camPos[1], camPos[2], attrCount, tCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(visibleOut, viewOut, viewMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		zClip(viewOut, clippedOrig, zClipOut, attrCount, tCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(clippedOrig, output, projMat, attrCount, tCount, 0);
		multiply(zClipOut, output2, projMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		centerPoints(output, output, screenWidth, screenHeight, attrCount, tCount);
		centerPoints(output2, output2, screenWidth, screenHeight, attrCount, tCount);
		printf("(%f, %f, %f)  (%f, %f, %f)  (%f, %f)\n", output[0], output[1], output[2],
		output[3], output[4], output[5], output[6], output[7]);
		barrier(CLK_GLOBAL_MEM_FENCE);

		shadeTrigs(output, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2],
		lightParams[0], lightParams[1], lightParams[2],
		aLightParams[0], aLightParams[1], aLightParams[2], attrCount);
		shadeTrigs(output2, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2],
		lightParams[0], lightParams[1], lightParams[2],
		aLightParams[0], aLightParams[1], aLightParams[2], attrCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		/*fillTrigs(output, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		fillTrigs(output2, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		drawTrigs(output, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		drawTrigs(output2, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		barrier(CLK_GLOBAL_MEM_FENCE);*/
}

__kernel void fillTrigsKernel(
	__global float* input, __global float* output,
	int attrCount, int tCount,
	__global float* output2, __global float* zClipOut, __global float* clippedOrig,
	__global int* lineParams, __global float* scaleMat, __global float* scaledOut,
	__global float* rotMat, __global float* rotOut,
	__global float* transMat, __global float* transOut,
	__global float* viewMat, __global float* viewOut,
	__global float* projMat,
	__global int* screen, int screenWidth, int screenHeight,
	__global float* camPos, __global float* visibleOut) {

		multiply(input, scaledOut, scaleMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(scaledOut, rotOut, rotMat, attrCount, tCount, 1);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(rotOut, transOut, transMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		visibleTrigs(transOut, visibleOut, camPos[0], camPos[1], camPos[2], attrCount, tCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(visibleOut, viewOut, viewMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		zClip(viewOut, clippedOrig, zClipOut, attrCount, tCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(clippedOrig, output, projMat, attrCount, tCount, 0);
		multiply(zClipOut, output2, projMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		centerPoints(output, output, screenWidth, screenHeight, attrCount, tCount);
		centerPoints(output2, output2, screenWidth, screenHeight, attrCount, tCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		fillTrigs(output, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		fillTrigs(output2, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		drawTrigs(output, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		drawTrigs(output2, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		barrier(CLK_GLOBAL_MEM_FENCE);
}

__kernel void drawTrigsKernel(
	__global float* input, __global float* output,
	int attrCount, int tCount,
	__global float* output2, __global float* zClipOut, __global float* clippedOrig,
	__global int* lineParams, __global float* scaleMat, __global float* scaledOut,
	__global float* rotMat, __global float* rotOut,
	__global float* transMat, __global float* transOut,
	__global float* viewMat, __global float* viewOut,
	__global float* projMat,
	__global int* screen, int screenWidth, int screenHeight,
	__global float* camPos, __global float* visibleOut) {
		multiply(input, scaledOut, scaleMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(scaledOut, rotOut, rotMat, attrCount, tCount, 1);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(rotOut, transOut, transMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		visibleTrigs(transOut, visibleOut, camPos[0], camPos[1], camPos[2], attrCount, tCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(visibleOut, viewOut, viewMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		zClip(viewOut, clippedOrig, zClipOut, attrCount, tCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(clippedOrig, output, projMat, attrCount, tCount, 0);
		multiply(zClipOut, output2, projMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		centerPoints(output, output, screenWidth, screenHeight, attrCount, tCount);
		centerPoints(output2, output2, screenWidth, screenHeight, attrCount, tCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		drawTrigs(output, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		drawTrigs(output2, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		barrier(CLK_GLOBAL_MEM_FENCE);
}

__kernel void drawWireframeDots(
	__global float* input, __global float* output,
	int attrCount, int tCount,
	__global int* lineParams, __global int* dotParams,
	__global float* scaleMat, __global float* rotMat,
	__global float* transMat, __global float* viewMat,
	__global float* projMat,
	__global float* scaledOut, __global float* rotOut,
	__global float* transOut, __global float* viewOut,
	__global float* zClipOut,
	__global int* screen, int screenWidth, int screenHeight, int dots,
	__global float* clippedOrig, __global float* output2) {

		multiply(input, scaledOut, scaleMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(scaledOut, rotOut, rotMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(rotOut, transOut, transMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(transOut, viewOut, viewMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		zClip(viewOut, clippedOrig, zClipOut, attrCount, tCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		multiply(clippedOrig, output, projMat, attrCount, tCount, 0);
		multiply(zClipOut, output2, projMat, attrCount, tCount, 0);
		barrier(CLK_GLOBAL_MEM_FENCE);

		centerPoints(output, output, screenWidth, screenHeight, attrCount, tCount);
		centerPoints(output2, output2, screenWidth, screenHeight, attrCount, tCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		drawTrigs(output, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		drawTrigs(output2, screen, screenWidth, screenHeight, tCount,
		lineParams[0], lineParams[1], lineParams[2], attrCount);
		barrier(CLK_GLOBAL_MEM_FENCE);

		if (dots == 1) {
			drawPoints(output, screen, screenWidth, screenHeight,
			dotParams[0], dotParams[1], dotParams[2], dotParams[3], attrCount,
			tCount);
		}
	}
