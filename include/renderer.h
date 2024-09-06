#ifndef RENDERER_H
#define RENDERER_H

#include "cglm/cglm.h"
#include "shader.h"

#define MAX_TITLE_LEN   256

struct Renderer
{
    unsigned int width, height;
    char title[MAX_TITLE_LEN];
};

struct Renderer seel_renderer_create(const char *title, unsigned int width, unsigned int height);
void seel_renderer_destroy(struct Renderer *renderer);
void seel_renderer_begin_frame(struct Renderer *renderer);
void seel_renderer_end_frame(struct Renderer *renderer);
void seel_renderer_set_clear_color(struct Renderer *renderer, vec3 color);

#endif /* RENDERER_H */