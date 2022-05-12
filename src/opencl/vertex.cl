__kernel void multiplyPoints(__global float* points, __global float* output,
										__global float* m) {
	int i = get_global_id(0);
	int w;
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
	}
}

__kernel void drawPoints(__global int* points, __global int* output,
											int screenWidth) {
	int i = get_global_id(0);
	if (i % 3 == 0) {
		int x = points[i];
		int y = points[i+1];
		output[(y*(screenWidth))+x] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
	}
}
