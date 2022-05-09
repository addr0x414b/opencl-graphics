__kernel void test(__global int* points, __global int* output) {
	int i = get_global_id(0);
	output[i] = points[i];
}
