// YUV444p is same as RGB, YUV are both taking 1 bits.
// so a yuv444p is 24 bits.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// num - Number of frames to process
int simplest_yuv444p_split(char* url, int width, int height, int num)
{
  FILE *fp = fopen(url, "rb+");
  FILE *y_fp = fopen("output_444p_y.dat", "wb+");
  FILE *u_fp = fopen("output_444p_u.dat", "wb+");
  FILE *v_fp = fopen("output_444p_v.dat", "wb+");

  size_t width_mul_height = width * height;
  size_t total_size = width_mul_height * 3;

  uint8_t *pic = (uint8_t *) malloc(total_size);

  for (int i = 0; i < num; i++) {
    fread(pic, 1, total_size, fp);

    // Y
    fwrite(pic, 1, width_mul_height, y_fp);

    // U
    fwrite(pic + width_mul_height, 1, width_mul_height, u_fp);

    // V
    fwrite(pic + width_mul_height * 2, 1, width_mul_height, v_fp);
  }

  free(pic);
  pic = NULL;
  fclose(fp);
  fclose(y_fp);
  fclose(u_fp);
  fclose(v_fp);

  return 0;
}

int main(int argc, char** argv)
{
  simplest_yuv444p_split("lena_256x256_yuv444p.yuv", 256, 256, 1);
  return 0;
}