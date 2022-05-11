__kernel void perspectiveMult(__global float* points, __global int* output) {

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
