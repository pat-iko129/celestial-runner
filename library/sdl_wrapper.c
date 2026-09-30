#include "sdl_wrapper.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_image.h>
#include <assert.h>
#include <math.h>
#include <state.h>
#include <stdlib.h>
#include <time.h>

#include <stdbool.h>
//#include "SDL_thread.h"
#include <SDL2/SDL_mixer.h>

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  case SDLK_SPACE:
    return UP_ARROW;
  case SDLK_p:
    return P;
  case SDL_MOUSEMOTION:
    return MOUSE_MOTIONS;
  case SDL_BUTTON_LEFT:
    return MOUSE_LEFT;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}
// case SDL_MOUSEMOTION:
//   printf("a;lskdjf;alksjdf\n");
//   return MOUSE_MOTIONS;

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);

  // int imgFlags = IMG_INIT_JPG || IMG_INIT_PNG;
  // IMG_Init(imgFlags);
  // Mix_Init(0);

  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
}

bool sdl_is_done(state_t *state) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);

  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      free(event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      // Skip the keypress if no handler is configured
      // or an unrecognized key was pressed
      if (key_handler == NULL)
        break;
      char key = get_keycode(event->key.keysym.sym);
      if (key == '\0')
        break;

      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type =
          event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler(key, type, held_time, state);
      break;
    }
  }
  free(event);
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);
  assert(0 <= color.r && color.r <= 1);
  assert(0 <= color.g && color.g <= 1);
  assert(0 <= color.b && color.b <= 1);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);
  free(x_points);
  free(y_points);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene) {
  // sdl_clear();
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    sdl_draw_polygon(shape, body_get_color(body));
    list_free(shape);
  }
  // sdl_show();
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}

SDL_Texture *create_text(TTF_Font *font, char *text) {
  // this opens a font style and sets a size
  // TTF_Font* Sans = TTF_OpenFont("../assets/comici.ttf", 64);
  // if (!Sans){
  //   printf("bruh");
  // }

  // this is the color in rgb format,
  // maxing out all would give you the color white,
  // and it will be your text's color

  // if (font == NULL) {
  //   font = TTF_OpenFont("assets/comici.ttf", 90);
  //   if (font == NULL) {
  //     printf("emily2 %s\n", TTF_GetError());
  //     printf("EMILY IS SO cool\n");
  //   }
  // }

  SDL_Color White = {255, 255, 255};

  // as TTF_RenderText_Solid could only be used on
  // SDL_Surface then you have to create the surface first
  SDL_Surface *surfaceMessage = TTF_RenderText_Solid(font, text, White);

  // now you can convert it into a texture
  SDL_Texture *Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

  SDL_Rect *Message_rect = malloc(sizeof(SDL_Rect)); // create a rect
  Message_rect->x = 0;                 // controls the rect's x coordinate
  Message_rect->y = 10;                // controls the rect's y coordinte
  Message_rect->w = surfaceMessage->w; // controls the width of the rect
  Message_rect->h = surfaceMessage->h; // controls the height of the rect

  // (0,0) is on the top left of the window/screen,
  // think a rect as the text's box,
  // that way it would be very simple to understand

  // Now since it's a texture, you have to put RenderCopy
  // in your game loop area, the area where the whole code executes

  // you put the renderer's name first, the Message,
  // the crop size (you can ignore this if you don't want
  // to dabble with cropping), and the rect which is the size
  // and coordinate of your texture
  SDL_RenderCopy(renderer, Message, NULL, Message_rect);
  // SDL_RenderPresent(renderer);

  // Don't forget to free your surface and texture
  SDL_FreeSurface(surfaceMessage);
  // SDL_DestroyTexture(Message);
  free(Message_rect);
}

void sound_effect() {
  // Initialize mixer
  Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 4096);
  Mix_AllocateChannels(100);
  Mix_Chunk *laserCollide = Mix_LoadWAV("assets/Laser.wav");
  if (!laserCollide) {
    printf("Laser collide sound not loaded properly.\n");
  }
  Mix_PlayChannel(3, laserCollide, 0);
  // -1 channel is first free channel, background music can use 3
}

SDL_Texture *screen_load() {
  int flags = IMG_INIT_PNG;
  int initted = IMG_Init(flags);
  if ((initted & flags) != flags) {
    printf("IMG_Init: Failed to init required jpg and png support!\n");
    printf("hiIMG_Init: %s\n", IMG_GetError());
    // handle error
  }

  SDL_Surface *image = IMG_Load("assets/sample.png");
  SDL_Texture *image_texture = SDL_CreateTextureFromSurface(renderer, image);
  // printf("IMG_Load error: %s\n", IMG_GetError());

  SDL_Rect *image_rect = malloc(sizeof(SDL_Rect)); // create a rect
  image_rect->x = 0;        // controls the rect's x coordinate
  image_rect->y = 0;        // controls the rect's y coordinte
  image_rect->w = image->w; // controls the width of the rect
  image_rect->h = image->h;

  SDL_RenderCopy(renderer, image_texture, NULL, image_rect);
  SDL_FreeSurface(image);
  // SDL_DestroyTexture(image_texture);
  return image_texture;
}