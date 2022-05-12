__kernel void perspectiveMult(__global float* points, __global float* output) {
	int i = get_global_id(0);
	float w;
	float fovRad = (60.f/2.f) * (3.141592f / 180.f);
	float aspect = 640 / 480;
	float zNear = 0.1f;
	float zFar = 1000.f;

	float zz = (1.f / (tan(fovRad))) / aspect;
	float oo = 1.f / tan(fovRad);
	float tt = ((-2.f * zNear) / (zFar - zNear)) - 1.f;
	float tht = (-zNear * zFar) / (zFar - zNear);
	float tth = -1.0f;
	if (i % 3 == 0) {
		output[i] = (points[i] * zz);
	}
	if (i % 3 == 1) {
		output[i] = points[i] * oo;

	}
	if (i % 3 == 2) {
		output[i] = points[i] * tt + tth;
		w = points[i] * tht;
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
