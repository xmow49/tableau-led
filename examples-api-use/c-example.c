/* -*- mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *
 * Using the C-API of this library.
 *
 */
#include "led-matrix-c.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <time.h>

void my_delay(int i) /*Pause l'application pour i seconds*/
{
  clock_t start, end;
  start = clock();
  while (((end = clock()) - start) <= i * CLOCKS_PER_SEC)
    ;
}

int main(int argc, char **argv)
{
  struct RGBLedMatrixOptions options;
  struct RGBLedMatrix *matrix;
  struct LedCanvas *offscreen_canvas;
  int width, height;
  int x, y, i;

  memset(&options, 0, sizeof(options));
  options.rows = 64;
  options.cols = 64;
  options.chain_length = 2;
  options.parallel = 2;
  options.show_refresh_rate = true;
  options.brightness = 100;

  /* This supports all the led commandline options. Try --led-help */
  matrix = led_matrix_create_from_options(&options, &argc, &argv);
  if (matrix == NULL)
    return 1;

  /* Let's do an example with double-buffering. We create one extra
   * buffer onto which we draw, which is then swapped on each refresh.
   * This is typically a good aproach for animations and such.
   */
  offscreen_canvas = led_matrix_create_offscreen_canvas(matrix);

  led_canvas_get_size(offscreen_canvas, &width, &height);

  fprintf(stderr, "Size: %dx%d. Hardware gpio mapping: %s\n",
          width, height, options.hardware_mapping);

  /* Let's do an example with double-buffering. We create one extra
   * buffer onto which we draw, which is then swapped on each refresh.
   * This is typically a good aproach for animations and such.
   */
  offscreen_canvas = led_matrix_create_offscreen_canvas(matrix);

  led_canvas_get_size(offscreen_canvas, &width, &height);

  fprintf(stderr, "Size: %dx%d. Hardware gpio mapping: %s\n",
          width, height, options.hardware_mapping);

  while (1)
  {
    // for (i = 0; i < 1000; ++i)
    // {
    //   for (y = 0; y < height; ++y)
    //   {
    //     for (x = 0; x < width; ++x)
    //     {
    //       led_canvas_set_pixel(offscreen_canvas, x, y, i & 0xff, x, y);
    //     }
    //   }

    //   offscreen_canvas = led_matrix_swap_on_vsync(matrix, offscreen_canvas);
    // }

    led_canvas_fill(offscreen_canvas, 255, 255, 255);
    offscreen_canvas = led_matrix_swap_on_vsync(matrix, offscreen_canvas);

  }

  /*
   * Make sure to always call led_matrix_delete() in the end to reset the
   * display. Installing signal handlers for defined exit is a good idea.
   */
  led_matrix_delete(matrix);
  return 0;
}
