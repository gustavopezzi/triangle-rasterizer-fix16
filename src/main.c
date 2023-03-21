#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "display.h"
#include "vec2.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

bool is_running = false;

vec2_t vertices[4] = {
  { .x = 40 * 0x00010000, .y = 40 * 0x00010000 }, // values manually converted to 16.16 fixed point
  { .x = 80 * 0x00010000, .y = 40 * 0x00010000 }, // values manually converted to 16.16 fixed point
  { .x = 40 * 0x00010000, .y = 80 * 0x00010000 }, // values manually converted to 16.16 fixed point
  { .x = 90 * 0x00010000, .y = 90 * 0x00010000 }, // values manually converted to 16.16 fixed point
};

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} color_t;

color_t colors[3] = {
  { .r = 0xFF, .g = 0x00, .b = 0x00 },
  { .r = 0x00, .g = 0xFF, .b = 0x00 },
  { .r = 0x00, .g = 0x00, .b = 0xFF }
};

void process_input(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        is_running = false;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
          is_running = false;
        break;
    }
  }
}

bool is_top_left(vec2_t* start, vec2_t* end) {
  vec2_t edge = { fix16_sub(end->x, start->x), fix16_sub(end->y, start->y) };
  bool is_top_edge = edge.y == 0 && edge.x > 0;
  bool is_left_edge = edge.y < 0;
  return is_left_edge || is_top_edge;
}

fix16_t edge_cross(vec2_t* a, vec2_t* b, vec2_t* p) {
  vec2_t ab = { fix16_sub(b->x, a->x), fix16_sub(b->y, a->y) };
  vec2_t ap = { fix16_sub(p->x, a->x), fix16_sub(p->y, a->y) };
  return fix16_sub(fix16_mul(ab.x, ap.y), fix16_mul(ab.y, ap.x));
}

int count = 0;

void triangle_fill(vec2_t v0, vec2_t v1, vec2_t v2) {
  // Finds the bounding box with all candidate pixels
  int x_min = fix16_to_int(MIN(MIN(v0.x, v1.x), v2.x));
  int y_min = fix16_to_int(MIN(MIN(v0.y, v1.y), v2.y));
  int x_max = fix16_to_int(MAX(MAX(v0.x, v1.x), v2.x));
  int y_max = fix16_to_int(MAX(MAX(v0.y, v1.y), v2.y));

  // Compute the area of the entire triangle/parallelogram
  fix16_t area = edge_cross(&v0, &v1, &v2);

  // Compute the constant delta_s that will be used for the horizontal and vertical steps
  fix16_t delta_w0_col = fix16_sub(v1.y, v2.y);
  fix16_t delta_w1_col = fix16_sub(v2.y, v0.y);
  fix16_t delta_w2_col = fix16_sub(v0.y, v1.y);
  fix16_t delta_w0_row = fix16_sub(v2.x, v1.x);
  fix16_t delta_w1_row = fix16_sub(v0.x, v2.x);
  fix16_t delta_w2_row = fix16_sub(v1.x, v0.x);

  // Rasterization fill convention (top-left rule)
  fix16_t bias0 = is_top_left(&v1, &v2) ? 0 : -0x00000001;
  fix16_t bias1 = is_top_left(&v2, &v0) ? 0 : -0x00000001;
  fix16_t bias2 = is_top_left(&v0, &v1) ? 0 : -0x00000001;

  // Compute the edge functions for the fist (top-left) point
  vec2_t p0 = { fix16_from_float(x_min + 0.5f) , fix16_from_float(y_min + 0.5f) };
  fix16_t w0_row = fix16_add(edge_cross(&v1, &v2, &p0), bias0);
  fix16_t w1_row = fix16_add(edge_cross(&v2, &v0, &p0), bias1);
  fix16_t w2_row = fix16_add(edge_cross(&v0, &v1, &p0), bias2);

  // Loop all candidate pixels inside the bounding box
  for (int y = y_min; y <= y_max; y++) {
    fix16_t w0 = w0_row;
    fix16_t w1 = w1_row;
    fix16_t w2 = w2_row;
    
    for (int x = x_min; x <= x_max; x++) {
      bool is_inside = w0 >= 0 && w1 >= 0 && w2 >= 0;
      if (is_inside) {
        // Compute an interpolation between the colors of vertices v0, v1, and v2 using barycentric weights
        fix16_t alpha = fix16_div(w0, area);
        fix16_t beta = fix16_div(w1, area);
        fix16_t gamma = fix16_div(w2, area);

        int a = 0xFF;
        int r = fix16_to_float(alpha) * colors[0].r + fix16_to_float(beta) * colors[1].r + fix16_to_float(gamma) * colors[2].r;
        int g = fix16_to_float(alpha) * colors[0].g + fix16_to_float(beta) * colors[1].g + fix16_to_float(gamma) * colors[2].g;
        int b = fix16_to_float(alpha) * colors[0].b + fix16_to_float(beta) * colors[1].b + fix16_to_float(gamma) * colors[2].b;

        // Combine A, R, G, and B into one final 32-bit color
        uint32_t interp_color = 0x00000000;
        interp_color = (interp_color | a) << 8;
        interp_color = (interp_color | b) << 8;
        interp_color = (interp_color | g) << 8;
        interp_color = (interp_color | r);

        draw_pixel(x, y, interp_color);
      }
      // Increment one step to the right
      w0 = fix16_add(w0, delta_w0_col);
      w1 = fix16_add(w1, delta_w1_col);
      w2 = fix16_add(w2, delta_w2_col);
    }
    // Increment one row step
    w0_row = fix16_add(w0_row, delta_w0_row);
    w1_row = fix16_add(w1_row, delta_w1_row);
    w2_row = fix16_add(w2_row, delta_w2_row);
  }
}

void render(void) {
  clear_framebuffer(0xFF000000);

  fix16_t angle = fix16_from_float(SDL_GetTicks() / 1000.0f * 0.2);

  vec2_t center = {
    fix16_from_float(SCREEN_WIDTH / 2.0f),
    fix16_from_float(SCREEN_HEIGHT / 2.0f)
  };

  vec2_t v0 = vec2_rotate(vertices[0], center, angle);
  vec2_t v1 = vec2_rotate(vertices[1], center, angle);
  vec2_t v2 = vec2_rotate(vertices[2], center, angle);
  vec2_t v3 = vec2_rotate(vertices[3], center, angle);

  triangle_fill(v0, v1, v2);
  triangle_fill(v3, v2, v1);

  render_framebuffer();
}

int main(void) {
  is_running = create_window();

  while (is_running) {
    fix_framerate();
    process_input();
    render();
  }

  destroy_window();

  return EXIT_SUCCESS;
}
