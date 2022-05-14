__kernel void helloWorld(__global int* input, __global int* output) {
	int i = get_global_id(0);

	if (i == 0) {
		output[i] = input[0] + input[1];
	}
	if (i == 1) {
		output[i] = input[2] + input[3];
	}
	if (i == 2) {
		output[i] = input[4] + input[5];
	}

}
