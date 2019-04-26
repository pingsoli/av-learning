// From the source code, you can know the basic format of yuv420p.
// the yuv picture total size: width * height * 3 / 2
// y [0, w * h)
// u [w * h, w * h * 5/4)
// v [w * h * 5/4, w * h * 3/2]

// How to obtain the yuv picture from the video file ?
// 1. get yuv420p rawvideo first
// ffmpeg -i thor_1280x720.mp4 -c:v rawvideo -pix_fmt thor_1280x720.yuv
//
// 2. Capture first picture frome the video
//
// 3. Convert a jpg picture to yuv raw picture
// ffmpeg -i input.jpg -f rawvideo -pix_fmt yuv420p output.yuv

// Using yuvplayer from leixiaohua1020 github repo.
// link address: https://github.com/leixiaohua1020/simplest_mediadata_test

// NOTE: U, V must based on Y, so make sure you use the correct pixel format.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// num - Number of frames to process
int simplest_yuv420p_split(char* url, int width, int height, int num)
{
  FILE *fp = fopen(url, "rb+");
  FILE *y_fp = fopen("output_420p_y.dat", "wb+");
  FILE *u_fp = fopen("output_420p_u.dat", "wb+");
  FILE *v_fp = fopen("output_420p_v.dat", "wb+");

  size_t width_mul_height = width * height;
  size_t total_size = width_mul_height * 3/2;
  size_t uv_size = width_mul_height / 4;

  uint8_t *pic = (uint8_t *) malloc(total_size);

  for (int i = 0; i < num; i++) {
    fread(pic, 1, total_size, fp);

    // Y
    fwrite(pic, 1, width_mul_height, y_fp);

    // U
    fwrite(pic + width_mul_height, 1, uv_size, u_fp);

    // V
    fwrite(pic + (width_mul_height * 5/4), 1, uv_size, v_fp);
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
  // simplest_yuv420p_split("lena_256x256_yuv420p.yuv", 256, 256, 1);
  simplest_yuv420p_split("thor_1280x720_yuv420p.yuv", 1280, 720, 1);
  return 0;
}