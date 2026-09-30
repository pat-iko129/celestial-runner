#ifndef __COLOR_H__
#define __COLOR_H__

/**
 * A color to display on the screen.
 * The color is represented by its red, green, and blue components.
 * Each component must be between 0 (black) and 1 (white).
 */
typedef struct {
  float r;
  float g;
  float b;
} rgb_color_t;

extern const rgb_color_t YELLOW;
extern const rgb_color_t RED;
extern const rgb_color_t BLUE;

float max(float a, float b);
float min(float a, float b);

void rainbows(rgb_color_t rgb, float inc);

#endif // #ifndef __COLOR_H__
