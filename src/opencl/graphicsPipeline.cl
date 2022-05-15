void drawPoints(float* input, int* screen, int screenWidth, int screenHeight) {
	int i = get_global_id(0);
	if (i % 3 == 0) {
		int maxSize = screenWidth * screenHeight;
		int x = (int)round(input[i]);
		int y = (int)round(input[i+1]);
		if (x >= 0 && x <= maxSize && y >= 0 && y <= maxSize) {
			screen[y*(screenWidth)+x] = 0xFF000000 | (255 << 16) | (255 << 8)|(255);
		}
	}
}

void centerPoints(float* input, float* output, int screenWidth, int screenHeight){
	int i = get_global_id(0);

	if (i % 3 == 0) {
		float x = input[i] * ((float)screenWidth / (float)screenHeight);
		output[i] = x + ((float)screenWidth / 2.0f);
	}
	if (i % 3 == 1) {
		output[i] = input[i] + ((float)screenHeight / 2.0f);
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
			output[i+2] /= w;
		}
	}
}

__kernel void submitVertices(__global float* input, __global float* output,
										__global int* screen, int screenWidth,
										int screenHeight, int pointCount) {
	int i = get_global_id(0);

	float fovRad = (60.0f/2.0f) * (3.141592f / 180.0f);
	float aspect = (float)screenWidth / (float)screenHeight;

	float m[16];
	m[0] = (1.0f / tan(fovRad)) / aspect;
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = -(1.0f / tan(fovRad));
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = ((-2.0f * 0.1f) / (1000.0f - 0.1f)) - 1.0f;
	m[11] = -1.0f;

	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = (-0.1f * 1000.0f) / (1000.0f - 0.1f);
	m[15] = 0.0f;

	multiply(input, output, m);
	barrier(CLK_LOCAL_MEM_FENCE);

	centerPoints(output, output, screenWidth, screenHeight);
	barrier(CLK_LOCAL_MEM_FENCE);

	drawPoints(output, screen, screenWidth, screenHeight);
	barrier(CLK_LOCAL_MEM_FENCE);

	printf("Distance: %f\n", output[3] - output[0]);
}
