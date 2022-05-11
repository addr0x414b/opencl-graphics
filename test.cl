__kernel void test(__global float* points, __global int* output) {
	int i = get_global_id(0);
	if ((i) % 3 == 0) {
		output[i] = (points[i]*320) + 640/2;
	}
	if ((i) % 3 == 1){
		output[i] = -(points[i]*240) + 480/2;
	}
}

__kernel void test2(__global int* points, __global int* output) {
	int i = get_global_id(0);
	int x;
	int y;
	if ((i) % 3 == 0) {
		x = points[i];
		y = points[i+1];
		printf("THIS: %d, %d\n", x, y);
		output[(y*640) + x] = 0xFF000000 | (255 << 16) | (255 << 8) | (255);
	}
	if ((i+1) % 2 == 0) {
	}
}
