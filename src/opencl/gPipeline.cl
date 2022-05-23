void drawPixel(int x, int y, int* screen, int screenWidth,
		int screenHeight, int r, int g, int b) {
	int maxSize = screenWidth * screenHeight;
	if (x >= 0 && x <= maxSize && y >= 0 && y <= maxSize) {
		screen[y*(screenWidth)+x] = 0xFF000000 | (r << 16) | (g << 8) | b;
	}
}

void drawLine(int x1, int y1, int x2, int y2, int* screen, int screenWidth,
		int screenHeight, int r, int g, int b) {
	if (x1 == x2) {
		if (y1 == y2) {
			drawPixel(x1, y1, screen, screenWidth, screenHeight, r, g, b);
		} else {
			if (y1 > y2) {
				int temp = y1;
				y1 = y2;
				y2 = temp;
			}
			for (int y = y1; y <= y2; y++) {
				drawPixel(x1, y, screen, screenWidth, screenHeight, r, g, b);
			}
		}
	} else if (y1 == y2) {
		if (x1 > x2) {
			int temp = x1;
			x1 = x2;
			x2 = temp;
		}
		for (int x = x1; x <= x2; x++) {
			drawPixel(x, y1, screen, screenWidth, screenHeight, r, g, b);
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
			drawPixel(x, y, screen, screenWidth, screenHeight, r, g, b);
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
			drawPixel(x, y, screen, screenWidth, screenHeight, r, g, b);
		}
	}
}

void drawPoints(float* input, int* screen, int screenWidth, int screenHeight,
		int r, int g, int b, int thickness, int attrCount) {

	int i = get_global_id(0);
	if (i % attrCount == 0) {
		int maxSize = screenWidth * screenHeight;
		int x = (int)round(input[i]);
		int y = (int)round(input[i+1]);
		for (int j = -thickness; j <= thickness; j++) {
			drawLine(x-thickness, y+j, x+thickness, y+j, screen, screenWidth,
						screenHeight, r, g, b);
		}
	}
}

void drawTrigs(float* input, int* screen, int screenWidth, int screenHeight,
		int pointCount, int r, int g, int b, int attrCount) {
	int i = get_global_id(0);

	if (i % (attrCount*3) == 0 && i % attrCount == 0) {
		int x1 = (int)round(input[i]);
		int y1 = (int)round(input[i+1]);

		int x2 = (int)round(input[i+attrCount]);
		int y2 = (int)round(input[i+attrCount+1]);

		int x3 = (int)round(input[i+attrCount*2]);
		int y3 = (int)round(input[i+attrCount*2 + 1]);

		drawLine(x1, y1, x2, y2, screen, screenWidth, screenHeight, r, g, b);
		drawLine(x2, y2, x3, y3, screen, screenWidth, screenHeight, r, g, b);
		drawLine(x3, y3, x1, y1, screen, screenWidth, screenHeight, r, g, b);
	}
}

void centerPoints(float* input, float* output, int screenWidth,
		int screenHeight, int attrCount){

	int i = get_global_id(0);

	if (i % attrCount == 0) {
		output[i] = input[i] * 1920.0f/4.0f;
		output[i] += (float)screenWidth / 2.0f;
	}
	if (i % attrCount == 1) {
		output[i] = -input[i] * 1920.0f/4.0f;
		output[i] += (float)screenHeight / 2.0f;
	}

}

void multiply(float* input, float* output, float* m, int attrCount) {

	int i = get_global_id(0);


	if (i % attrCount == 0) {
		output[i] = (input[i] * m[0]) + (input[i+1] * m[1]) + (input[i+2] * m[2])
			+ m[3];
	}

	if (i % attrCount == 1) {
		output[i] = (input[i-1] * m[4]) + (input[i] * m[5]) + (input[i+1] * m[6])
			+ m[7];
	}

	if (i % attrCount == 2) {
		output[i] = (input[i-2] * m[8]) + (input[i-1] * m[9]) + (input[i] * m[10])
			+ m[11];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	if (i % attrCount == 0) {
		float w = (input[i] * m[12]) + (input[i+1] * m[13]) + (input[i+2] * m[14])
			+ m[15];
		if (w != 0.0f) {
			output[i] /= w;
			output[i+1] /= w;
			output[i+2] = output[i+2] / w;
		}
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	if (attrCount > 3) {
		if(i % attrCount > 2) {
			output[i] = input[i];
		}
	}
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
		__global int* screen, int screenWidth, int screenHeight, int dots) {

	int i = get_global_id(0);


	multiply(input, scaledOut, scaleMat, attrCount);
	barrier(CLK_LOCAL_MEM_FENCE);

	multiply(scaledOut, rotOut, rotMat, attrCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	multiply(rotOut, transOut, transMat, attrCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	multiply(transOut, viewOut, viewMat, attrCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	multiply(viewOut, output, projMat, attrCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	centerPoints(output, output, screenWidth, screenHeight, attrCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	drawTrigs(output, screen, screenWidth, screenHeight, tCount,
			lineParams[0], lineParams[1], lineParams[2], attrCount);
	barrier(CLK_LOCAL_MEM_FENCE);

	if (dots == 1) {
		drawPoints(output, screen, screenWidth, screenHeight,
				dotParams[0], dotParams[1], dotParams[2], dotParams[3], attrCount);
	}
}
