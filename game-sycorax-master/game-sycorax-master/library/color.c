#include "color.h"
#include "math.h"
#include <stdio.h>

const rgb_color_t YELLOW = {1., 1., 0.};
const rgb_color_t RED = {1., 0., 0.};
const rgb_color_t BLUE = {0., 0., 1.};

float max(float d1, float d2) {
  if (d1 > d2) {
    return d1;
  } else {
    return d2;
  }
}

float min(float d1, float d2) {
  if (d1 < d2) {
    return d1;
  } else {
    return d2;
  }
}

float hue(rgb_color_t rgb, float max, float min) {
  float range = max - min;
  float hue_val = 0.;
  if (range == 0.) {
    return 0;
  }
  if (max == rgb.r) {
    hue_val = (rgb.g - rgb.b) / range;
    hue_val = fmod(hue_val, 6);
  } else if (max == rgb.b) {
    hue_val = 4 + (rgb.r - rgb.g) / range;
  } else if (max == rgb.g) {
    hue_val = 2 + (rgb.b - rgb.r) / range;
  } else {
    return -1;
  }
  hue_val = hue_val * (float)(M_PI / 3);
  return hue_val;
}

void rgb_to_hsv(rgb_color_t rgb) {
  float r = rgb.r;
  float b = rgb.b;
  float g = rgb.g;

  float fmax = max(max(r, b), g);
  float fmin = min(min(r, b), g);

  float h = hue((rgb_color_t){r, g, b}, fmax, fmin);
  float s;
  if (fmax != 0) {
    s = (fmax - fmin) / fmax;
  } else {
    s = 0;
  }
  float v = fmax;

  rgb.r = h;
  rgb.g = s;
  rgb.b = v;
}

void hsv_to_rgb(rgb_color_t rgb) {

  float h = rgb.r;
  float s = rgb.g;
  float v = rgb.b;

  float c = v * s;
  float h_prime = h * M_PI / 3;

  float x = c * (1 - fabs(fmod((double)h_prime, 2) - 1));

  rgb_color_t rgb1;

  if (h_prime >= 0 && h_prime < 1) {
    rgb1.r = c;
    rgb1.g = x;
    rgb1.b = 0;
  } else if (h_prime >= 1 && h_prime < 2) {
    rgb1.r = x;
    rgb1.g = c;
    rgb1.b = 0;
  } else if (h_prime >= 2 && h_prime < 3) {
    rgb1.r = 0;
    rgb1.g = c;
    rgb1.b = x;
  } else if (h_prime >= 3 && h_prime < 4) {
    rgb1.r = 0;
    rgb1.g = x;
    rgb1.b = c;

  } else if (h_prime >= 4 && h_prime < 5) {
    rgb1.r = x;
    rgb1.g = 0;
    rgb1.b = c;
  } else if (h_prime >= 5 && h_prime < 6) {
    rgb1.r = c;
    rgb1.g = 0;
    rgb1.b = x;
  }

  float m = v - c;
  rgb.r = rgb1.r + m;
  rgb.g = rgb1.g + m;
  rgb.b = rgb1.b + m;
}

void rainbows(rgb_color_t rgb, float inc) {
  rgb_to_hsv(rgb);
  rgb.r = (rgb.r + inc);
  hsv_to_rgb(rgb);
}