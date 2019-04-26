#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int simplest_yuv420p_y_half(char *url, int width, int height, int num)
{
  size_t total_size = width * height * 3/2;
  size_t width_mul_height = width * height;

  FILE *fp = fopen(url, "rb+");
  FILE *y_half_fp = fopen("lena_output_yuv420p_y_half.yuv", "wb+");

  uint8_t* pic = (uint8_t *) malloc(total_size);

  for (int i = 0; i < num; i++) {
    fread(pic, 1, total_size, fp);

    for (unsigned int j = 0; j < width_mul_height; j++)
      pic[j] /= 2;

    fwrite(pic, 1, total_size, y_half_fp);
  }

  free(pic);
  fclose(fp);
  fclose(y_half_fp);

  return 0;
}

int main(int argc, char **argv) {
  simplest_yuv420p_y_half("lena_256x256_yuv420p.yuv", 256, 256, 1);
  return 0;
}