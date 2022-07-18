
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
      if (x != (int)screenWidth/2 && y != (int)screenHeight/2 && x > 0 && x < screenWidth && y > 0 && y < screenHeight) {
        for (int j = -thickness; j <= thickness; j++) {
          drawLine(x-thickness, y+j, x+thickness, y+j, screen, screenWidth,
          screenHeight, r, g, b);
        }
      }
    }
  }
}

void sort2D(int* x1, int* y1, int* x2, int* y2, int* x3, int* y3, int n, int gl, int xy) {

  int* points[6];
  points[0] = x1; points[1] = y1;
  points[2] = x2; points[3] = y2;
  points[4] = x3; points[5] = y3;

  if (gl == 1) {
    if (xy == 0) {
      if (*x1 <= *x2 && *x2 <= *x3) {
        return;
      } else {
        while (*points[0] > *points[2] || *points[0] > *points[4] || *points[2] > *points[4]) {
          for (int i = 0; i <= 2; i += 2) {
            if (*points[i] > *points[i+2]) {
              int tx = *points[i];
              int ty = *points[i+1];

              *points[i] = *points[i+2];
              *points[i+1] = *points[i+3];

              *points[i+2] = tx;
              *points[i+3] = ty;
            }
          }
        }
      }
    } else if (xy == 1) {
      if (*y1 <= *y2 && *y2 <= *y3) {
        return;
      } else {
        while (*points[1] > *points[3] || *points[1] > *points[5] || *points[3] > *points[5]) {
          for (int i = 1; i <= 3; i += 2) {
            if (*points[i] > *points[i+2]) {
              int tx = *points[i-1];
              int ty = *points[i];

              *points[i-1] = *points[i+1];
              *points[i] = *points[i+2];

              *points[i+1] = tx;
              *points[i+2] = ty;
            }
          }
        }
      }
    }
  } else if (gl == 0) {
    if (xy == 0) {
      if (*x1 >= *x2 && *x2 >= *x3) {
        return;
      } else {
        while (*points[0] < *points[2] || *points[0] < *points[4] || *points[2] < *points[4]) {
          for (int i = 0; i <= 2; i += 2) {
            if (*points[i] < *points[i+2]) {
              int tx = *points[i];
              int ty = *points[i+1];

              *points[i] = *points[i+2];
              *points[i+1] = *points[i+3];

              *points[i+2] = tx;
              *points[i+3] = ty;
            }
          }
        }
      }
    } else if (xy == 1) {
      if (*y1 >= *y2 && *y2 >= *y3) {
        return;
      } else {
        while (*points[1] < *points[3] || *points[1] < *points[5] || *points[3] < *points[5]) {
          for (int i = 1; i <= 3; i += 2) {
            if (*points[i] < *points[i+2]) {
              int tx = *points[i-1];
              int ty = *points[i];

              *points[i-1] = *points[i+1];
              *points[i] = *points[i+2];

              *points[i+1] = tx;
              *points[i+2] = ty;
            }
          }
        }
      }
    }
  }
}

void clipTop(int x1, int y1, int x2, int y2, int x3, int y3, int* screen,
int screenWidth, int screenHeight, int r, int g, int b) {

  int clipAmt = 1;
  if (y1 > clipAmt && y2 > clipAmt && y3 > clipAmt) {
    drawLine(x1, y1, x2, y2, screen, screenWidth, screenHeight, r, g, b);
    drawLine(x2, y2, x3, y3, screen, screenWidth, screenHeight, r, g, b);
    drawLine(x3, y3, x1, y1, screen, screenWidth, screenHeight, r, g, b);
  } else if (y1 < clipAmt && y2 < clipAmt && y3 < clipAmt) {

  } else {
    sort2D(&x1, &y1, &x2, &y2, &x3, &y3, screenHeight, 0, 1);
    if (y2 > clipAmt) {
      int ax = 0;
      int ay = clipAmt;

      int bx = 0;
      int by = clipAmt;

      if (x3 != x1) {
        float s1 = (((float)y3 - (float)y1) / ((float)x3 - (float)x1));
        float b1 = ((float)y1 - (s1 * (float)x1));
        ax = (int)round(((float)ay - b1) / s1);
      } else {
        ax = x1;
      }

      if (x3 != x2) {
        float s2 = (((float)y3 - (float)y2) / ((float)x3 - (float)x2));
        float b2 = ((float)y3 - (s2 * (float)x3));
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

      if (x3 != x1) {
        float s1 = (((float)y3 - (float)y1) / ((float)x3 - (float)x1));
        float b1 = ((float)y1 - (s1 * (float)x1));
        ax = (int)round(((float)ay - b1) / s1);
      } else {
        ax = x1;
      }

      if (x2 != x1) {
        float s2 = (((float)y2 - (float)y1) / ((float)x2 - (float)x1));
        float b2 = ((float)y1 - (s2 * (float)x1));
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

void clipLeft(int x1, int y1, int x2, int y2, int x3, int y3, int* screen,
int screenWidth, int screenHeight, int r, int g, int b) {

  int clipAmt = 1;
  if (x1 > clipAmt && x2 > clipAmt && x3 > clipAmt) {
      clipTop(x1, y1, x2, y2, x3, y3, screen, screenWidth, screenHeight, r, g, b);
  } else if (x1 < clipAmt && x2 < clipAmt && x3 < clipAmt) {

  } else {
    sort2D(&x1, &y1, &x2, &y2, &x3, &y3, screenHeight, 0, 0);
    if (x2 > clipAmt) {
      int ax = clipAmt;
      int ay = 0;

      int bx = clipAmt;
      int by = 0;

      if (x3 != x1) {
        float s1 = (((float)y3 - (float)y1) / ((float)x3 - (float)x1));
        float b1 = ((float)y1 - (s1 * (float)x1));
        ay = (int)round(((s1*(float)ax) + b1));
      } else {
        ax = x1;
      }

      if (x3 != x2) {
        float s2 = (((float)y3 - (float)y2) / ((float)x3 - (float)x2));
        float b2 = ((float)y3 - (s2 * (float)x3));
        by = (int)round(((s2*(float)bx) + b2));
      } else {
        bx = x2;
      }

      clipTop(x1, y1, x2, y2, ax, ay, screen, screenWidth, screenHeight, r, g, b);
      clipTop(bx, by, x2, y2, ax, ay, screen, screenWidth, screenHeight, r, g, b);
    } else {
      int ax = clipAmt;
      int ay = 0;
      int bx = clipAmt;
      int by = 0;

      if (x3 != x1) {
        float s1 = (((float)y3 - (float)y1) / ((float)x3 - (float)x1));
        float b1 = ((float)y1 - (s1 * (float)x1));
        ay = (int)round(((s1*(float)ax) + b1));
      } else {
        ax = x1;
      }

      if (x2 != x1) {
        float s2 = (((float)y2 - (float)y1) / ((float)x2 - (float)x1));
        float b2 = ((float)y1 - (s2 * (float)x1));
        by = (int)round(((s2*(float)bx) + b2));
      } else {
        bx = x1;
      }

      clipTop(x1, y1, bx, by, ax, ay, screen, screenWidth, screenHeight, r, g, b);
    }

  }
}

void clipBot(int x1, int y1, int x2, int y2, int x3, int y3, int* screen,
int screenWidth, int screenHeight, int r, int g, int b) {

  int clipAmt = screenHeight - 1;
  if (y1 < clipAmt && y2 < clipAmt && y3 < clipAmt) {
      clipLeft(x1, y1, x2, y2, x3, y3, screen, screenWidth, screenHeight, r, g, b);
  } else if (y1 > clipAmt && y2 > clipAmt && y3 > clipAmt) {

  } else {
    sort2D(&x1, &y1, &x2, &y2, &x3, &y3, screenHeight, 1, 1);
    if (y2 < clipAmt) {
      int ax = 0;
      int ay = clipAmt;

      int bx = 0;
      int by = clipAmt;

      if (x3 != x1) {
        float s1 = (((float)y3 - (float)y1) / ((float)x3 - (float)x1));
        float b1 = ((float)y1 - (s1 * (float)x1));
        ax = (int)round(((float)ay - b1) / s1);
      } else {
        ax = x1;
      }

      if (x3 != x2) {
        float s2 = (((float)y3 - (float)y2) / ((float)x3 - (float)x2));
        float b2 = ((float)y3 - (s2 * (float)x3));
        bx = (int)round(((float)by - b2) / s2);
      } else {
        bx = x2;
      }

      clipLeft(x1, y1, x2, y2, ax, ay, screen, screenWidth, screenHeight, r, g, b);
      clipLeft(bx, by, x2, y2, ax, ay, screen, screenWidth, screenHeight, r, g, b);
    } else {
      int ax = 0;
      int ay = clipAmt;
      int bx = 0;
      int by = clipAmt;

      if (x3 != x1) {
        float s1 = (((float)y3 - (float)y1) / ((float)x3 - (float)x1));
        float b1 = ((float)y1 - (s1 * (float)x1));
        ax = (int)round(((float)ay - b1) / s1);
      } else {
        ax = x1;
      }

      if (x2 != x1) {
        float s2 = (((float)y2 - (float)y1) / ((float)x2 - (float)x1));
        float b2 = ((float)y1 - (s2 * (float)x1));
        bx = (int)round(((float)by - b2) / s2);
      } else {
        bx = x1;
      }

      clipLeft(x1, y1, bx, by, ax, ay, screen, screenWidth, screenHeight, r, g, b);
    }

  }
}

void clipDraw(int x1, int y1, int x2, int y2, int x3, int y3, int* screen,
int screenWidth, int screenHeight, int r, int g, int b) {

  int clipAmt = screenWidth - 1;
  if (x1 < clipAmt && x2 < clipAmt && x3 < clipAmt) {
      clipBot(x1, y1, x2, y2, x3, y3, screen, screenWidth, screenHeight, r, g, b);
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
    }
  }
}

void centerPoints(float* input, float* output, int screenWidth,
int screenHeight, int attrCount, int tCount){

  int i = get_global_id(0);

  if (i < tCount) {
    if (i % attrCount == 0) {
      if (input[i] == 0.0f) {
        return;
      }
      output[i] = input[i] * 1920.0f/4.0f;
      output[i] += (float)screenWidth / 2.0f;
    }
    if (i % attrCount == 1) {
      if (input[i] == 0.0f) {
        return;
      }
      output[i] = -input[i] * 1920.0f/4.0f;
      output[i] += (float)screenHeight / 2.0f;
    }
  }
}

void multiply(float* input, float* output, float* m, int attrCount, int tCount, int norms) {

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

      if (norms == 1) {
        output[i+3] = (input[i+3] * m[0]) + (input[i+4] * m[1]) + (input[i+5] * m[2])
        + m[3];
        output[i+4] = (input[i+3] * m[4]) + (input[i+4] * m[5]) + (input[i+5] * m[6])
        + m[7];
        output[i+5] = (input[i+3] * m[8]) + (input[i+4] * m[9]) + (input[i+5] * m[10])
        + m[11];
        float w = (input[i+3] * m[12]) + (input[i+4] * m[13]) + (input[i+5] * m[14])
        + m[15];
        if (w != 0.0f) {
          output[i+3] /= w;
          output[i+4] /= w;
          output[i+5] = output[i+5] / w;
        }

      }
    }

    if (norms == 0) {
      if (attrCount > 3) {
        if(i % attrCount > 2) {
          output[i] = input[i];
        }
      }
    }
  }
}

void zClip(float* input, float* orig, float* clipped, int attrCount, int tCount) {
  int i = get_global_id(0);

  if (i < tCount) {
    if (i % (attrCount*3) == 0 && i % attrCount == 0) {
      float clip = -5.f;

      float x1 = input[i];
      float y1 = input[i+1];
      float z1 = input[i+2];

      float x2 = input[i+attrCount];
      float y2 = input[i+attrCount+1];
      float z2 = input[i+attrCount+2];

      float x3 = input[i+attrCount*2];
      float y3 = input[i+attrCount*2 + 1];
      float z3 = input[i+attrCount*2 + 2];

      if (x1 == x2 && x1 == x3 && x2 == x3) {
        return;
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
      } else {
        float* points[9];
        points[0] = &x1; points[1] = &y1; points[2] = &z1;
        points[3] = &x2; points[4] = &y2; points[5] = &z2;
        points[6] = &x3; points[7] = &y3; points[8] = &z3;

        while (*points[2] < *points[5] || *points[2] < *points[8] || *points[5] < *points[8]) {
          for (int i = 2; i <= 5; i += 3) {
            if (*points[i] < *points[i+3]) {
              float tx = *points[i+1];
              float ty = *points[i+2];
              float tz = *points[i+3];

              *points[i+1] = *points[i-2];
              *points[i+2] = *points[i-1];
              *points[i+3] = *points[i];

              *points[i-2] = tx;
              *points[i-1] = ty;
              *points[i] = tz;
            }
          }
        }
        if (z1 > clip && z2 < clip && z3 < clip) {
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
}

void visibleTrigs(__global float* input, __global float* output, float camX,
float camY, float camZ, int attrCount, int tCount) {
  int i = get_global_id(0);

  if (i < tCount) {
    if (i % (attrCount*3) == 0 && i % attrCount == 0) {

      float dirX = input[i] - camX;
      float dirY = input[i+1] - camY;
      float dirZ = input[i+2] - camZ;

      float dot = (input[i+3] * dirX) + (input[i+4] * dirY) + (input[i+5] * dirZ);

      if (dot < 0.0f) {
        output[i] = input[i];
        output[i+1] = input[i+1];
        output[i+2] = input[i+2];

        output[i+3] = input[i+3];
        output[i+4] = input[i+4];
        output[i+5] = input[i+5];

        output[i+6] = input[i+6];
        output[i+7] = input[i+7];
        output[i+8] = input[i+8];

        output[i+9] = input[i+9];
        output[i+10] = input[i+10];
        output[i+11] = input[i+11];

        output[i+12] = input[i+12];
        output[i+13] = input[i+13];
        output[i+14] = input[i+14];

        output[i+15] = input[i+15];
        output[i+16] = input[i+16];
        output[i+17] = input[i+17];
      }
    }
    /*if (attrCount > 3) {
       if(i % attrCount > 2) {
         output[i] = input[i];
       }
     }*/
  }
}
