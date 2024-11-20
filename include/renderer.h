#ifndef RENDERER_H
#define RENDERER_H

#include <stdio.h>
#include <string.h>

#include "GLFW/glfw3.h"
#include "glad/gl.h"

#include "cglm/cglm.h"
#include "shader.h"
#include "light.h"
#include "camera.h"

#define MAX_TITLE_LEN 256

struct Renderer
{
    unsigned int width, height;
    char title[MAX_TITLE_LEN];
    struct Shader active_shader;
    struct DirectionalLight *dir_lights;
    struct PointLights *point_lights;
    struct SpotLights *spot_lights;
    struct Camera camera;
};

struct Renderer seel_renderer_create(const char *title, unsigned int width, unsigned int height);
void seel_renderer_destroy(struct Renderer *renderer);
void seel_renderer_begin_frame(struct Renderer *renderer);
void seel_renderer_end_frame(struct Renderer *renderer);
void seel_renderer_set_clear_color(struct Renderer *renderer, vec3 color);

struct Renderer seel_renderer_create(const char *title, unsigned int width, unsigned int height)
{
    struct Renderer renderer = {0};
    renderer.width = width;
    renderer.height = height;
    strcpy(renderer.title, title);
    renderer.dir_lights = NULL;
    renderer.point_lights = NULL;
    renderer.spot_lights = NULL;

    return renderer;
}

void seel_renderer_render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
}

void seel_renderer_destroy(struct Renderer *renderer);

#endif /* RENDERER_H */