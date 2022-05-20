void drawPixel(int x, int y, int* screen, int screenWidth,
					int screenHeight, int r, int g, int b) {
	int maxSize = screenWidth * screenHeight;
	if (x >= 0 && x <= maxSize && y >= 0 && y <= maxSize) {
		screen[y*(screenWidth)+x] = 0xFF000000 | (r << 16) | (g << 8) | b;
	}
}

void drawLine(int x1, int y1, int x2, int y2, int* screen, int screenWidth,
					int screenHeight, int r, int g, int b) {
	int thickness = 0;
	if (x1 == x2) {
		if (y1 == y2) {
			for(int i = -thickness; i <= thickness; i++) {
				drawPixel(x1, y1+i, screen, screenWidth, screenHeight, r, g, b);
			}
		} else {
			if (y1 > y2) {
				int temp = y1;
				y1 = y2;
				y2 = temp;
			}
			for (int y = y1; y <= y2; y++) {
				for(int i = -thickness; i <= thickness; i++) {
					drawPixel(x1, y+i, screen, screenWidth, screenHeight, r, g, b);
				}
			}
		}
	} else if (y1 == y2) {
		if (x1 > x2) {
			int temp = x1;
			x1 = x2;
			x2 = temp;
		}
		for (int x = x1; x <= x2; x++) {
			for(int i = -thickness; i <= thickness; i++) {
				drawPixel(x, y1+i, screen, screenWidth, screenHeight, r, g, b);
			}
		}
	} else {
		if (x1 > x2) {
			int temp = x1;
			x1 = x2;
			x2 = temp;
			int temp2 = y1;
			y1 = y2;
			y2 = temp2;
		}
		float m = ((float)y2 - (float)y1) / ((float)x2 - (float)x1);
		float intercept = ((m * (float)x1) - (float)y1) / -1.0f;
		for (int x = x1; x <= x2; x++) {
			int y = (int)round(((m * (float)x) + intercept));
			for(int i = -thickness; i <= thickness; i++) {
				drawPixel(x, y+i, screen, screenWidth, screenHeight, r, g, b);
			}
		}

		if (y1 > y2) {
			int temp = y1;
			y1 = y2;
			y2 = temp;
			int temp2 = x1;
			x1 = x2;
			x2 = temp2;
		}
		for (int y = y1; y <= y2; y++) {
			int x = (int)round(((float)y - intercept) / m);
			for(int i = -thickness; i <= thickness; i++) {
				drawPixel(x, y+i, screen, screenWidth, screenHeight, r, g, b);
			}
		}
	}
}

void drawPoints(float* input, int* screen, int screenWidth, int screenHeight) {

	int i = get_global_id(0);
	if (i % 3 == 0) {
		int maxSize = screenWidth * screenHeight;
		int x = (int)round(input[i]);
		int y = (int)round(input[i+1]);
		int thickness = 3;
		for (int j = -thickness; j <= thickness; j++) {
			drawLine(x-thickness, y+j, x+thickness, y+j, screen, screenWidth,
						screenHeight, 255, 255, 255);
		}
	}
}

void drawTrigs(float* input, int* screen, int screenWidth, int screenHeight,
					int pointCount) {
	int i = get_global_id(0);

	if (i % 9 == 0) {
		int x1 = (int)round(input[i]);
		int y1 = (int)round(input[i+1]);

		int x2 = (int)round(input[i+3]);
		int y2 = (int)round(input[i+4]);

		int x3 = (int)round(input[i+6]);
		int y3 = (int)round(input[i+7]);

		drawLine(x1, y1, x2, y2, screen, screenWidth, screenHeight, 255, 255, 0);
		drawLine(x2, y2, x3, y3, screen, screenWidth, screenHeight, 255, 255, 0);
		drawLine(x3, y3, x1, y1, screen, screenWidth, screenHeight, 255, 255, 0);
	}
}

void centerPoints(float* input, float* output, int screenWidth,
		int screenHeight){

	int i = get_global_id(0);

	if (i % 3 == 0) {
		//float x = input[i] * ((float)screenWidth / (float)screenHeight);
		//output[i] = x + ((float)screenWidth / 2.0f);
		//output[i] = input[i] + 50.0f;
		//output[i] *= 0.01f * (float)screenWidth;
		output[i] = input[i] * 150.0f;
		output[i] += (float)screenWidth / 2.0f;
	}
	if (i % 3 == 1) {
		//output[i] = input[i] + ((float)screenHeight / 2.0f);
		//output[i] = input[i] + 50.0f;
		//output[i] *= 0.01f * (float)screenHeight;
		output[i] = -input[i] * 150.0f;
		output[i] += (float)screenHeight / 2.0f;
	}

}

void multiply(float* input, float* output, float* m) {

	int i = get_global_id(0);


	if (i % 3 == 0) {
		output[i] = (input[i] * m[0]) + (input[i+1] * m[1]) + (input[i+2] * m[2])
			+ m[3];
	}

	if (i % 3 == 1) {
		output[i] = (input[i-1] * m[4]) + (input[i] * m[5]) + (input[i+1] * m[6])
			+ m[7];
	}

	if (i % 3 == 2) {
		output[i] = (input[i-2] * m[8]) + (input[i-1] * m[9]) + (input[i] * m[10])
			+ m[11];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	if (i % 3 == 0) {
		float w = (input[i] * m[12]) + (input[i+1] * m[13]) + (input[i+2] * m[14])
			+ m[15];
		if (w != 0.0f) {
			output[i] /= w;
			output[i+1] /= w;
			output[i+2] = output[i+2] / w;
		}
	}
}

__kernel void drawWireframeDots(
		__global float* input, __global float* output,
		int attrCount, int tCount,
		__global int* lineParams, __global int* dotParams,
		__global float* scaleMat, __global float* rotMat,
		__global float* transMat, __global float* projMat,
		__global float* scaledOut, __global float* rotOut,
		__global float* transOut,
		__global int* screen, int screenWidth, int screenHeight) {

	int i = get_global_id(0);

	multiply(input, scaledOut, scaleMat);
	barrier(CLK_LOCAL_MEM_FENCE);

	multiply(scaledOut, rotOut, rotMat);
	barrier(CLK_GLOBAL_MEM_FENCE);

	multiply(rotOut, transOut, transMat);
	barrier(CLK_GLOBAL_MEM_FENCE);

	multiply(transOut, output, projMat);
	barrier(CLK_GLOBAL_MEM_FENCE);

	if ( i == 0) {
		printf("INPUT: (%f, %f, %f)\n", input[0], input[1], input[2]);
		printf("GPU Scaled: (%f, %f, %f)\n", scaledOut[0], scaledOut[1], scaledOut[2]);
		printf("GPU Rotated: (%f, %f, %f)\n", rotOut[0], rotOut[1], rotOut[2]);
		printf("GPU Translated: (%f, %f, %f)\n",
				 transOut[0], transOut[1], transOut[2]);
		printf("GPU Projected: (%f, %f, %f)\n", output[0], output[1], output[2]);
	}


	centerPoints(output, output, screenWidth, screenHeight);
	barrier(CLK_GLOBAL_MEM_FENCE);

	drawTrigs(output, screen, screenWidth, screenHeight, tCount);
	barrier(CLK_LOCAL_MEM_FENCE);

	drawPoints(output, screen, screenWidth, screenHeight);
}

__kernel void submitVertices(__global float* input, __global float* output,
		__global int* screen, int screenWidth,
		int screenHeight, int pointCount, __global float* projMat,
		__global float* scaleMat, __global float* rotMat,
		__global float* transMat, __global float* scaled,
		__global float* rotated, __global float* translated) {

	int i = get_global_id(0);

	multiply(input, scaled, scaleMat);
	barrier(CLK_LOCAL_MEM_FENCE);

	multiply(scaled, rotated, rotMat);
	barrier(CLK_GLOBAL_MEM_FENCE);

	multiply(rotated, translated, transMat);
	barrier(CLK_GLOBAL_MEM_FENCE);

	multiply(translated, output, projMat);
	barrier(CLK_GLOBAL_MEM_FENCE);

	if ( i == 0) {
		//printf("INPUT: (%f, %f, %f)\n", input[0], input[1], input[2]);
		//printf("GPU Scaled: (%f, %f, %f)\n", scaled[0], scaled[1], scaled[2]);
		//printf("GPU Rotated: (%f, %f, %f)\n", rotated[0], rotated[1], rotated[2]);
		//printf("GPU Translated: (%f, %f, %f)\n",
		//		 translated[0], translated[1], translated[2]);
		//printf("GPU Projected: (%f, %f, %f)\n", output[0], output[1], output[2]);
	}


	centerPoints(output, output, screenWidth, screenHeight);
	barrier(CLK_GLOBAL_MEM_FENCE);

	drawTrigs(output, screen, screenWidth, screenHeight, pointCount);
	barrier(CLK_LOCAL_MEM_FENCE);

	drawPoints(output, screen, screenWidth, screenHeight);
}
