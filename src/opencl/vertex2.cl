__kernel void multiplyPoints(__global float* points, __global float* output,
										__global float* m) {
	int i = get_global_id(0);
	printf("%d\n", i);
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
