void drawPixel(int x, int y, int* screen, int screenWidth,
		int screenHeight, int r, int g, int b) {
	int maxSize = screenWidth * screenHeight;
	if (x >= 0 && x <= maxSize && y >= 0 && y <= maxSize) {
		screen[y*(screenWidth)+x] = 0xFF000000 | (r << 16) | (g << 8) | b;
	}
}

void drawLine(int x1, int y1, int x2, int y2, int* screen, int screenWidth,
		int screenHeight, int r, int g, int b) {
	if (x1 == x2) {
		if (y1 == y2) {
			drawPixel(x1, y1, screen, screenWidth, screenHeight, r, g, b);
		} else {
			if (y1 > y2) {
				int temp = y1;
				y1 = y2;
				y2 = temp;
			}
			for (int y = y1; y <= y2; y++) {
				drawPixel(x1, y, screen, screenWidth, screenHeight, r, g, b);
			}
		}
	} else if (y1 == y2) {
		if (x1 > x2) {
			int temp = x1;
			x1 = x2;
			x2 = temp;
		}
		for (int x = x1; x <= x2; x++) {
			drawPixel(x, y1, screen, screenWidth, screenHeight, r, g, b);
		}
	} else {
		if (x1 > x2) {
			int temp = x1;
			x1 = x2;
			x2 = temp;
			int temp2 = y1;
			y1 = y2;
			y2 = temp2;
		}
		float m = ((float)y2 - (float)y1) / ((float)x2 - (float)x1);
		float intercept = ((m * (float)x1) - (float)y1) / -1.0f;
		for (int x = x1; x <= x2; x++) {
			int y = (int)round(((m * (float)x) + intercept));
			drawPixel(x, y, screen, screenWidth, screenHeight, r, g, b);
		}

		if (y1 > y2) {
			int temp = y1;
			y1 = y2;
			y2 = temp;
			int temp2 = x1;
			x1 = x2;
			x2 = temp2;
		}
		for (int y = y1; y <= y2; y++) {
			int x = (int)round(((float)y - intercept) / m);
			drawPixel(x, y, screen, screenWidth, screenHeight, r, g, b);
		}
	}
}

void drawPoints(float* input, int* screen, int screenWidth, int screenHeight,
		int r, int g, int b, int thickness, int attrCount, int tCount) {

	int i = get_global_id(0);
	if (i < tCount) {
		if (i % attrCount == 0) {
			int maxSize = screenWidth * screenHeight;
			int x = (int)round(input[i]);
			int y = (int)round(input[i+1]);
			if (x != (int)screenWidth/2 && y != (int)screenHeight/2) {
				for (int j = -thickness; j <= thickness; j++) {
					drawLine(x-thickness, y+j, x+thickness, y+j, screen, screenWidth,
								screenHeight, r, g, b);
				}
			}
		}
	}
}

void sort2D(int* x1, int* y1, int* x2, int* y2, int* x3, int* y3, int n, int gl, int xy) {
	if (gl == 1) {
		if (xy == 0) {
			if (*x1 <= *x2 && *x2 <= *x3) {
				return;
			} else {
				if (*x1 <= *x2 && *x1 <= *x3) {
					if (*x3 <= *x2) {
						int tx = *x3;
						int ty = *y3;

						*x3 = *x2;
						*y3 = *y2;

						*x2 = tx;
						*y2 = ty;
					}
				} else if (*x2 <= *x1 && *x2 <= *x3) {
					int tx = *x1;
					int ty = *y1;

					*x1 = *x2;
					*y1 = *y2;

					*x2 = tx;
					*y2 = ty;

					if (*x3 <= *x2) {
						tx = *x3;
						ty = *y3;

						*x3 = *x2;
						*y3 = *y2;

						*x2 = tx;
						*y2 = ty;
					}
				} else if (*x3 <= *x1 && *x3 <= *x2) {
					int tx = *x1;
					int ty = *y1;

					*x1 = *x3;
					*y1 = *y3;

					*x3 = tx;
					*y3 = ty;

					if (*x3 <= *x2) {
						tx = *x3;
						ty = *y3;

						*x3 = *x2;
						*y3 = *y2;

						*x2 = tx;
						*y2 = ty;
					}
				}
			}
		} else if (xy == 1) {
			if (*y1 <= *y2 && *y2 <= *y3) {
				return;
			} else {
				if (*y1 <= *y2 && *y1 <= *y3) {
					if (*y3 <= *y2) {
						int tx = *x3;
						int ty = *y3;

						*x3 = *x2;
						*y3 = *y2;

						*x2 = tx;
						*y2 = ty;
					}
				} else if (*y2 <= *y1 && *y2 <= *y3) {
					int tx = *x1;
					int ty = *y1;

					*x1 = *x2;
					*y1 = *y2;

					*x2 = tx;
					*y2 = ty;

					if (*y3 <= *y2) {
						tx = *x3;
						ty = *y3;

						*x3 = *x2;
						*y3 = *y2;

						*x2 = tx;
						*y2 = ty;
					}
				} else if (*y3 <= *y1 && *y3 <= *y2) {
					int tx = *x1;
					int ty = *y1;

					*x1 = *x3;
					*y1 = *y3;

					*x3 = tx;
					*y3 = ty;

					if (*y3 <= *y2) {
						tx = *x3;
						ty = *y3;

						*x3 = *x2;
						*y3 = *y2;

						*x2 = tx;
						*y2 = ty;
					}
				}
			}
		}

	}
}

void clipBot(int x1, int y1, int x2, int y2, int x3, int y3, int* screen,
		int screenWidth, int screenHeight, int r, int g, int b) {

	int clipAmt = screenHeight - 1;
	if (y1 < clipAmt && y2 < clipAmt && y3 < clipAmt) {
		if (x1 != screenWidth/2 && y1 != screenHeight/2) {
			drawLine(x1, y1, x2, y2, screen, screenWidth, screenHeight, r, g, b);
			drawLine(x2, y2, x3, y3, screen, screenWidth, screenHeight, r, g, b);
			drawLine(x3, y3, x1, y1, screen, screenWidth, screenHeight, r, g, b);
		}
	} else if (y1 > clipAmt && y2 > clipAmt && y3 > clipAmt) {

	} else {
		sort2D(&x1, &y1, &x2, &y2, &x3, &y3, screenHeight, 1, 1);
		//printf("(%d, %d), (%d, %d), (%d, %d))\n", x1, y1, x2, y2, x3, y3);
		if (y2 < clipAmt) {
			int ax = 0;
			int ay = clipAmt;

			int bx = 0;
			int by = clipAmt;
			if (y1 == y3) {
				//ay = y1;
			}
			if (x3 != x1) {
				float s1 = (((float)y3 - (float)y1) / ((float)x3 - (float)x1));
				float b1 = ((float)y1 - (s1 * (float)x1));
				//ay = (int)round(((s1*(float)ax) + b1));
				ax = (int)round(((float)ay - b1) / s1);
			} else {
				ax = x1;
			}

			if (y2 == y3) {
				//by = y2;
			}

			if (x3 != x2) {
				float s2 = (((float)y3 - (float)y2) / ((float)x3 - (float)x2));
				float b2 = ((float)y3 - (s2 * (float)x3));
				//by = (int)round(((s2*(float)bx) + b2));
				bx = (int)round(((float)by - b2) / s2);
			} else {
				bx = x2;
			}


			drawLine(x1, y1, ax, ay, screen, screenWidth, screenHeight, r, g, b);
			drawLine(ax, ay, x2, y2, screen, screenWidth, screenHeight, r, g, b);
			drawLine(x2, y2, x1, y1, screen, screenWidth, screenHeight, r, g, b);

			drawLine(ax, ay, bx, by, screen, screenWidth, screenHeight, r, g, b);
			drawLine(bx, by, x2, y2, screen, screenWidth, screenHeight, r, g, b);
			drawLine(x2, y2, ax, ay, screen, screenWidth, screenHeight, r, g, b);
		} else {
			int ax = 0;
			int ay = clipAmt;
			int bx = 0;
			int by = clipAmt;

			if (y1 == y3) {
				//ay = y1;
			}
			if (x3 != x1) {
				float s1 = (((float)y3 - (float)y1) / ((float)x3 - (float)x1));
				float b1 = ((float)y1 - (s1 * (float)x1));
				//ay = (int)round((s1*(float)ax) + b1);
				ax = (int)round(((float)ay - b1) / s1);
			} else {
				ax = x1;
			}

			if (y2 == y3) {
				//by = y2;
			}

			if (x2 != x1) {
				float s2 = (((float)y2 - (float)y1) / ((float)x2 - (float)x1));
				float b2 = ((float)y1 - (s2 * (float)x1));
				//by = (int)round((s2*(float)bx) + b2);
				bx = (int)round(((float)by - b2) / s2);
			} else {
				bx = x1;
			}
			

			drawLine(x1, y1, ax, ay, screen, screenWidth, screenHeight, r, g, b);
			drawLine(ax, ay, bx, by, screen, screenWidth, screenHeight, r, g, b);
			drawLine(bx, by, x1, y1, screen, screenWidth, screenHeight, r, g, b);
		}

	}
}

void clipDraw(int x1, int y1, int x2, int y2, int x3, int y3, int* screen,
		int screenWidth, int screenHeight, int r, int g, int b) {
	int clipAmt = screenWidth - 1;
	if (x1 < clipAmt && x2 < clipAmt && x3 < clipAmt) {
		if (x1 != screenWidth/2 && y1 != screenHeight/2) {
			clipBot(x1, y1, x2, y2, x3, y3, screen, screenWidth, screenHeight, r, g, b);
		}
	} else if (x1 > clipAmt && x2 > clipAmt && x3 > clipAmt) {
	} else {
		sort2D(&x1, &y1, &x2, &y2, &x3, &y3, screenWidth, 1, 0);
		if (x2 < clipAmt) {
			int ax = clipAmt;
			int ay = 0;

			int bx = clipAmt;
			int by = 0;
			if (y1 == y3) {
				ay = y1;
			} else {
				float s1 = (((float)y3 - (float)y1) / ((float)x3 - (float)x1));
				float b1 = ((float)y1 - (s1 * (float)x1));
				ay = (int)round(((s1*(float)ax) + b1));
			}

			if (y2 == y3) {
				by = y2;
			} else {
				float s2 = (((float)y3 - (float)y2) / ((float)x3 - (float)x2));
				float b2 = ((float)y3 - (s2 * (float)x3));
				by = (int)round(((s2*(float)bx) + b2));
			}


			clipBot(x1, y1, x2, y2, ax, ay, screen, screenWidth, screenHeight, r, g, b);
			clipBot(bx, by, x2, y2, ax, ay, screen, screenWidth, screenHeight, r, g, b);
			/*drawLine(x1, y1, ax, ay, screen, screenWidth, screenHeight, r, g, b);
			drawLine(ax, ay, x2, y2, screen, screenWidth, screenHeight, r, g, b);
			drawLine(x2, y2, x1, y1, screen, screenWidth, screenHeight, r, g, b);

			drawLine(ax, ay, bx, by, screen, screenWidth, screenHeight, r, g, b);
			drawLine(bx, by, x2, y2, screen, screenWidth, screenHeight, r, g, b);
			drawLine(x2, y2, ax, ay, screen, screenWidth, screenHeight, r, g, b);*/
		} else {
			int ax = clipAmt;
			int ay = 0;
			int bx = clipAmt;
			int by = 0;

			if (y1 == y3) {
				ay = y1;
			} else {
				float s1 = (((float)y3 - (float)y1) / ((float)x3 - (float)x1));
				float b1 = ((float)y1 - (s1 * (float)x1));
				ay = (int)round((s1*(float)ax) + b1);
			}

			if (y2 == y3) {
				by = y2;
			} else {
				float s2 = (((float)y2 - (float)y1) / ((float)x2 - (float)x1));
				float b2 = ((float)y1 - (s2 * (float)x1));
				by = (int)round((s2*(float)bx) + b2);
			}

			clipBot(x1, y1, bx, by, ax, ay, screen, screenWidth, screenHeight, r, g, b);
			/*drawLine(x1, y1, ax, ay, screen, screenWidth, screenHeight, r, g, b);
			drawLine(ax, ay, bx, by, screen, screenWidth, screenHeight, r, g, b);
			drawLine(bx, by, x1, y1, screen, screenWidth, screenHeight, r, g, b);*/
		}
	}
}

void drawTrigs(float* input, int* screen, int screenWidth, int screenHeight,
		int tCount, int r, int g, int b, int attrCount) {
	int i = get_global_id(0);

	if (i < tCount) {
		if (i % (attrCount*3) == 0 && i % attrCount == 0) {
			int x1 = (int)round(input[i]);
			int y1 = (int)round(input[i+1]);

			int x2 = (int)round(input[i+attrCount]);
			int y2 = (int)round(input[i+attrCount+1]);

			int x3 = (int)round(input[i+attrCount*2]);
			int y3 = (int)round(input[i+attrCount*2 + 1]);

			clipDraw(x1, y1, x2, y2, x3, y3, screen, screenWidth, screenHeight, r, g, b);

			/*if (x1 != screenWidth/2 && y1 != screenHeight/2) {
				drawLine(x1, y1, x2, y2, screen, screenWidth, screenHeight, r, g, b);
				drawLine(x2, y2, x3, y3, screen, screenWidth, screenHeight, r, g, b);
				drawLine(x3, y3, x1, y1, screen, screenWidth, screenHeight, r, g, b);

			}*/
		}
	}
}

void centerPoints(float* input, float* output, int screenWidth,
		int screenHeight, int attrCount, int tCount){

	int i = get_global_id(0);

	if (i < tCount) {
		if (i % attrCount == 0) {
			output[i] = input[i] * 1920.0f/4.0f;
			output[i] += (float)screenWidth / 2.0f;
		}
		if (i % attrCount == 1) {
			output[i] = -input[i] * 1920.0f/4.0f;
			output[i] += (float)screenHeight / 2.0f;
		}
	}
}

void multiply(float* input, float* output, float* m, int attrCount, int tCount) {

	int i = get_global_id(0);

	if (i < tCount) {
		if (i % attrCount == 0) {
			output[i] = (input[i] * m[0]) + (input[i+1] * m[1]) + (input[i+2] * m[2])
				+ m[3];
			output[i+1] = (input[i] * m[4]) + (input[i+1] * m[5]) + (input[i+2] * m[6])
				+ m[7];
			output[i+2] = (input[i] * m[8]) + (input[i+1] * m[9]) + (input[i+2] * m[10])
				+ m[11];
			float w = (input[i] * m[12]) + (input[i+1] * m[13]) + (input[i+2] * m[14])
				+ m[15];
			if (w != 0.0f) {
				output[i] /= w;
				output[i+1] /= w;
				output[i+2] = output[i+2] / w;
			}
		}

		if (attrCount > 3) {
			if(i % attrCount > 2) {
				output[i] = input[i];
			}
		}
	}
}

void add(__global int* extra) {
	//*extra += 9;
	atomic_add(extra, 9);
}

void zClip(float* input, float* orig, float* clipped, int attrCount, __global int* tCount) {
	int i = get_global_id(0);

	if (i < *tCount) {
		if (i % (attrCount*3) == 0 && i % attrCount == 0) {
			float clip = -5.f;

			//printf("%d\n", get_local_id(0));
			float x1 = input[i];
			float y1 = input[i+1];
			float z1 = input[i+2];

			float x2 = input[i+attrCount];
			float y2 = input[i+attrCount+1];
			float z2 = input[i+attrCount+2];

			float x3 = input[i+attrCount*2];
			float y3 = input[i+attrCount*2 + 1];
			float z3 = input[i+attrCount*2 + 2];

			if (z1 >= z2 && z1 >= z3) {
				if (z3 > z2) {
					float tx = x3;
					float ty = y3;
					float tz = z3;

					x3 = x2;
					y3 = y2;
					z3 = z2;

					x2 = tx;
					y2 = ty;
					z2 = tz;
				}
			} else if (z2 >= z1 && z2 >= z3) {
				if (z3 > z1) {
					float tx = x3;
					float ty = y3;
					float tz = z3;

					x3 = x1;
					y3 = y1;
					z3 = z1;

					x1 = tx;
					y1 = ty;
					z1 = tz;
				}
				float tx = x1;
				float ty = y1;
				float tz = z1;

				x1 = x2;
				y1 = y2;
				z1 = z2;

				x2 = tx;
				y2 = ty;
				z2 = tz;
			} else if (z3 >= z1 && z3 >= z2) {
				float tx = x1;
				float ty = y1;
				float tz = z1;

				x1 = x3;
				y1 = y3;
				z1 = z3;

				x3 = tx;
				y3 = ty;
				z3 = tz;
				if (z3 > z2) {
					float tx = x2;
					float ty = y2;
					float tz = z2;

					x2 = x3;
					y2 = y3;
					z2 = z3;

					x3 = tx;
					y3 = ty;
					z3 = tz;
				}
			}

			if (z1 < clip && z2 < clip && z3 < clip) {
				orig[i] = x1;
				orig[i+1] = y1;
				orig[i+2] = z1;

				orig[i+attrCount] = x2;
				orig[i+attrCount+1] = y2;
				orig[i+attrCount+2] = z2;

				orig[i+attrCount*2] = x3;
				orig[i+attrCount*2+1] = y3;
				orig[i+attrCount*2+2] = z3;
			} else if (z1 > clip && z2 < clip && z3 < clip) {
				float x2n = x2 - x1;
				float y2n = y2 - y1;
				float z2n = z2 - z1;

				float t2 = (clip - z1) / z2n;

				x2n = x1 + (x2n * t2);
				y2n = y1 + (y2n * t2);
				z2n = z1 + (z2n * t2);

				orig[i] = x2n;
				orig[i+1] = y2n;
				orig[i+2] = z2n;

				orig[i+attrCount] = x2;
				orig[i+attrCount+1] = y2;
				orig[i+attrCount+2] = z2;

				orig[i+attrCount*2] = x3;
				orig[i+attrCount*2+1] = y3;
				orig[i+attrCount*2+2] = z3;

				float x3n = x3 - x1;
				float y3n = y3 - y1;
				float z3n = z3 - z1;

				float t3 = (clip - z1) / z3n;

				x3n = x1 + (x3n * t3);
				y3n = y1 + (y3n * t3);
				z3n = z1 + (z3n * t3);

				clipped[i] = x2n;
				clipped[i+1] = y2n;
				clipped[i+2] = z2n;

				clipped[i+attrCount] = x3;
				clipped[i+attrCount+1] = y3;
				clipped[i+attrCount+2] = z3;

				clipped[i+attrCount*2] = x3n;
				clipped[i+attrCount*2+1] = y3n;
				clipped[i+attrCount*2+2] = z3n;

				//atomic_add(tCount, i * (*tCount));
				//atomic_add(tCount, 9);
			} else if (z1 > clip && z2 > clip && z3 < clip) {
				float x3n = x3 - x1;
				float y3n = y3 - y1;
				float z3n = z3 - z1;

				float t3 = (clip - z1) / z3n;

				x3n = x1 + (x3n * t3);
				y3n = y1 + (y3n * t3);
				z3n = z1 + (z3n * t3);

				float x2n = x3 - x2;
				float y2n = y3 - y2;
				float z2n = z3 - z2;

				float t2 = (clip - z2) / z2n;

				x2n = x2 + (x2n * t2);
				y2n = y2 + (y2n * t2);
				z2n = z2 + (z2n * t2);

				orig[i] = x2n;
				orig[i+1] = y2n;
				orig[i+2] = z2n;

				orig[i+attrCount] = x3n;
				orig[i+attrCount+1] = y3n;
				orig[i+attrCount+2] = z3n;

				orig[i+attrCount*2] = x3;
				orig[i+attrCount*2+1] = y3;
				orig[i+attrCount*2+2] = z3;


			}
		}
	}
}

void combineTrigs(float* orig, float* clipped, float* output, int n, int tCount) {
	int i = get_global_id(0);

	if (i < tCount) {
		if (i < n) {
			output[i] = orig[i];
		} else if(i >= n) {
			output[i] = clipped[i - n];
		}
	}

}

__kernel void drawWireframeDots(
		__global float* input, __global float* output,
		int attrCount, __global int* tCount,
		__global int* lineParams, __global int* dotParams,
		__global float* scaleMat, __global float* rotMat,
		__global float* transMat, __global float* viewMat,
		__global float* projMat,
		__global float* scaledOut, __global float* rotOut,
		__global float* transOut, __global float* viewOut,
		__global float* zClipOut,
		__global int* screen, int screenWidth, int screenHeight, int dots,
		__global float* clippedOrig, __global float* combined) {


	multiply(input, scaledOut, scaleMat, attrCount, *tCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	multiply(scaledOut, rotOut, rotMat, attrCount, *tCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	multiply(rotOut, transOut, transMat, attrCount, *tCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	multiply(transOut, viewOut, viewMat, attrCount, *tCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	zClip(viewOut, clippedOrig, zClipOut, attrCount, tCount);
	//printf("Test is: %d\n", clipExtra);
	//printf("%f, %f, %f\n", viewOut[0], viewOut[1], viewOut[2]);
	//printf("%f, %f, %f\n", viewOut[3], viewOut[4], viewOut[5]);
	//printf("%f, %f, %f\n", viewOut[6], viewOut[7], viewOut[8]);
	//tCount += 9;
	barrier(CLK_GLOBAL_MEM_FENCE);

	int n = *tCount;
	*tCount *= 2;

	combineTrigs(clippedOrig, zClipOut, combined, n, *tCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	//tCount = clipExtra;
	//*tCount += 5000;
	//printf("%f\n", zClipOut[19]);
	//printf("%d \n", *tCount);

	//printf("%d and %d\n", get_global_id(0), tCount);
	//multiply(viewOut, output, projMat, attrCount);
	//multiply(zClipOut, output, projMat, attrCount, *tCount);
	multiply(combined, output, projMat, attrCount, *tCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	centerPoints(output, output, screenWidth, screenHeight, attrCount, *tCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	drawTrigs(output, screen, screenWidth, screenHeight, *tCount,
			lineParams[0], lineParams[1], lineParams[2], attrCount);
	barrier(CLK_GLOBAL_MEM_FENCE);

	if (dots == 1) {
		drawPoints(output, screen, screenWidth, screenHeight,
			dotParams[0], dotParams[1], dotParams[2], dotParams[3], attrCount,
			*tCount);
	}
}
