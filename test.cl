__kernel void test(__global float* points, __global int* output) {
	int i = get_global_id(0);
	if ((i+1) % 2 != 0) {
		output[i] = points[i] * 640;
	}
	if ((i+1) % 2 == 0){
		output[i] = points[i] * 480;
	}
}

__kernel void test2(__global int* points, __global int* output) {
	int i = get_global_id(0);
	int x;
	int y;
	if ((i) % 2 == 0) {
		x = points[i];
		y = points[i+1];
		printf("THIS: %d, %d\n", x, y);
		output[(y*640) + x] = 0xFF000000 | 0x00FF0000 | 0x0000FF00
			| 0x000000FF;
	}
	if ((i+1) % 2 == 0) {
	}
}
