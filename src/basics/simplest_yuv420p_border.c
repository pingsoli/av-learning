#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int simplest_yuv420p_add_border(char *url, int width, int height, int border, int num) {
  FILE *fp = fopen(url, "rb+");
  FILE *add_border_fp = fopen("lena_output_border.yuv", "wb+");

  size_t width_mul_height = width * height;
  size_t total_size = width_mul_height * 3/2;

  uint8_t* pic = (uint8_t *) malloc(total_size);

  size_t new_width = width + border * 2;
  size_t new_height = height + border * 2;
  size_t new_width_mul_height = new_width * new_height;
  size_t new_total_size = new_width_mul_height * 3/2;
  uint8_t* new_pic = (uint8_t *) malloc(new_total_size);

  size_t y_start_pos = 0;
  size_t u_start_pos = new_width_mul_height;
  size_t v_start_pos = new_width_mul_height * 5/4;
  size_t uv_line_size = new_width / 4;

  for (int i = 0; i < num; i++) {
    fread(pic, 1, total_size, fp);

    // blank all
    memset(new_pic, 128, new_total_size);

    // fill the picture data
    for (int j = 0; j < height; j++) {
      memcpy(new_pic + (y_start_pos + new_width * (border + j) + border), pic + width * j, width); // y
      memcpy(new_pic + (u_start_pos + uv_line_size * (border + j) + border / 4), pic + width_mul_height + width * j, width / 4); // u
      memcpy(new_pic + (v_start_pos + uv_line_size * (border + j) + border / 4), pic + width_mul_height * 5/4 + width * j, width / 4); //v
    }

    fwrite(new_pic, 1, new_total_size, add_border_fp);
  }

  free(pic);
  free(new_pic);
  fclose(fp);
  fclose(add_border_fp);

  return 0;
}

int main(int argc, char** argv) {
  simplest_yuv420p_add_border("lena_256x256_yuv420p.yuv", 256, 256, 12, 1);
  return 0;
}