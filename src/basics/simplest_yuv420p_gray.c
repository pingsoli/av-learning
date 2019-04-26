
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int simplest_yuv420p_gray(char *url, int width, int height, int num)
{
  size_t width_mul_height = width * height;
  size_t total_size = width * height * 3/2;

  FILE *fp = fopen(url, "rb+");
  FILE *gray_fp = fopen("lena_output_grap.yuv", "wb+");
  uint8_t* pic = (uint8_t *) malloc(total_size);

  for (int i = 0; i < num; i++) {
    fread(pic, 1, total_size, fp);

    // Gray
    memset(pic + width_mul_height, 128, width_mul_height / 2);
    fwrite(pic, 1, width_mul_height * 3/2, gray_fp);
  }

  free(pic);
  fclose(fp);
  fclose(gray_fp);

  return 0;
}

int main(int argc, char **argv) {
  simplest_yuv420p_gray("lena_256x256_yuv420p.yuv", 256, 256, 1);
  return 0;
}