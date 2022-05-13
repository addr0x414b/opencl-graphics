__kernel void multiplyPoints(__global float* points, __global float* output,
										__global float* m) {
	int i = get_global_id(0);
	float w;
	if (i % 3 == 0) {
		output[i] = (points[i] * m[0]) + (points[i+1] * m[1]) + (points[i+2] * m[2]) + m[3];
	}
	if (i % 3 == 1) {
		output[i] = points[i-1] * m[4] + points[i] * m[5] + points[i+1] * m[6] + m[7];
	}
	if (i % 3 == 2) {
		output[i] = points[i-2] * m[8] + points[i-1] * m[9] + points[i] * m[10] + m[11];
		w = points[i-2] * m[12] + points[i-1] * m[13] + points[i] * m[14] + m[15];
		if (w != 0) {
			output[i] /= w;
			output[i-1] /= w;
			output[i-2] /= w;
		}
		printf("PROJECTION: index %d: %f, %f, %f | value: %f | w: %f\n", i, output[i-2], output[i-1], output[i], m[10], w);
	}
}

__kernel void centerFlipY(__global float* points, __global int* output,
								int screenWidth, int screenHeight) {
	int i = get_global_id(0);
	if (i % 3 == 0) {
		output[i] = (points[i]*(screenWidth/2)) + (screenWidth/2);
	}
	if (i % 3 == 1) {
		output[i] = -(points[i]*(screenHeight/2)) + (screenHeight/2);
	}
	if (i % 3 == 2) {
		output[i] = points[i];
		printf("FLIP: Index %d: %d, %d, %d\n", i, output[i-2], output[i-1], output[i]);
	}
}

__kernel void drawPoints(__global int* points, __global int* output,
											int screenWidth) {
	int i = get_global_id(0);

	if (i % 3 == 0) {
		int x = points[i];
		int y = points[i+1];
		output[(y*(screenWidth))+x] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y+1)*(screenWidth))+(x+1)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y)*(screenWidth))+(x+1)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y-1)*(screenWidth))+(x+1)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y-1)*(screenWidth))+(x)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y-1)*(screenWidth))+(x-1)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y)*(screenWidth))+(x-1)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y+1)*(screenWidth))+(x-1)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y+1)*(screenWidth))+(x)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);

		output[((y+2)*(screenWidth))+(x+2)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y)*(screenWidth))+(x+2)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y-2)*(screenWidth))+(x+2)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y-2)*(screenWidth))+(x)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y-2)*(screenWidth))+(x-2)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y)*(screenWidth))+(x-2)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y+2)*(screenWidth))+(x-2)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		output[((y+2)*(screenWidth))+(x)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
	}
}

void drawLine(int x1, int y1, int x2, int y2, int* output, int screenWidth) {
	printf("INPUTTED VALS: (%d, %d),  (%d, %d)\n", x1, y1, x2, y2);
	if (x1 == x2) {
		if (y1 == y2) {
		} else {
			if (y1 > y2) {
				int temp = y1;
				y1 = y2;
				y2 = temp;
			}
			for (int y = y1; y <= y2; y++) {
				output[((y)*(screenWidth))+(x1)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
			}
		}
	} else {
		float slope = (float)(y2 - y1) / (float)(x2 - x1);
		printf("SLOPE: %f | VALS: (%d - %d) / (%d - %d)\n", slope, y2, y1, x2, x1);
		float intercept = y1 - (slope * x1);
		if (x1 > x2) {
			int temp = x1;
			x1 = x2;
			x2 = temp;
		}
		for (int x = x1; x <= x2; x++) {
			int y = round((slope * x) + intercept);
			output[((y)*(screenWidth))+(x)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		}
		if (y1 > y2) {
			int temp = y1;
			y1 = y2;
			y2 = temp;
		}
		for (int y = y1; y <= y2; y++) {
			int x = round((y-intercept) / (float)slope);
			printf("CRASH: (%d, %d) WITH VALS: %f   %f\n", x, y, intercept, slope);
			output[((y)*(screenWidth))+(x)] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
		}
	}
}

__kernel void drawTrig(__global int* points, __global int* output,
								int screenWidth, int pointCount) {
	int i = get_global_id(0);

	if (i % 3 == 0 && pointCount - i >= 9) {
		printf("DRAW: Index %d: %d, %d, %d\n", i, points[i], points[i+1], points[i+2]);

		int x1 = points[i];
		int y1 = points[i+1];

		int x2 = points[i+3];
		int y2 = points[i+4];

		int x3 = points[i+6];
		int y3 = points[i+7];

		printf("TRIG POINTS: (%d, %d)  (%d, %d)  (%d, %d)\n", x1, y1, x2, y2, x3, y3);

		drawLine(x1, y1, x2, y2, output, screenWidth);
		drawLine(x2, y2, x3, y3, output, screenWidth);
		drawLine(x3, y3, x1, y1, output, screenWidth);

	}
}
