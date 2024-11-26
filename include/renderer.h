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
#include "error.h"
#include "model.h"
#include "light.h"

struct Renderer
{
    unsigned int width;
    unsigned int height;
    vec3 clear_color;
    unsigned int max_vertex_buffer;
    unsigned int max_element_buffer;
    bool enable_debug_output;
    bool enable_depth_test;
    bool enable_blending;
    struct Shader *active_shader;
    struct Camera *camera;
};

void seel_renderer_init(struct Renderer *renderer, struct RendererConfig *config, struct Camera *cam);
void seel_renderer_begin_frame(struct Renderer *renderer);
void seel_renderer_end_frame(void);
void seel_renderer_set_clear_color(struct Renderer *renderer, vec3 color);

void seel_renderer_init(struct Renderer *renderer, struct RendererConfig *config, struct Camera *cam)
{
    renderer->width = config->width;
    renderer->height = config->height;
    renderer->camera = cam;
    glm_vec3_copy(config->clear_color, renderer->clear_color);

    glViewport(0, 0, renderer->width, renderer->height);
    glClearColor(renderer->clear_color[0], renderer->clear_color[1], renderer->clear_color[2], 1.0f);

    if (config->enable_depth_test)
        glEnable(GL_DEPTH_TEST);
    if (config->enable_blending)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (config->enable_debug_output)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(seel_gl_debug_output, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
}

void seel_renderer_begin_frame(struct Renderer *renderer)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void seel_renderer_end_frame(void)
{
    glfwSwapBuffers(glfwGetCurrentContext());
    glfwPollEvents();
}

void seel_renderer_draw_model(struct Renderer *renderer, struct Model *model, mat4 model_matrix)
{
    if (!renderer->active_shader)
    {
        fprintf(stderr, "No active shader set for renderer.\n");
        return;
    }
    struct Shader *shader = renderer->active_shader;
    seel_shader_use(shader);

    /* Set matrices */
    mat4 view;
    seel_camera_get_view_matrix(renderer->camera, view);
    seel_shader_set_mat4(shader, "view", &view[0][0]);

    mat4 projection;
    glm_perspective(glm_rad(renderer->camera->zoom),
                    (float)renderer->width / (float)renderer->height,
                    renderer->camera->near_clip, renderer->camera->far_clip, projection);
    seel_shader_set_mat4(shader, "projection", &projection[0][0]);

    seel_shader_set_mat4(shader, "model", &model_matrix[0][0]);

    /* Set camera position */
    seel_shader_set_vec3(shader, "viewPos", renderer->camera->position);

    /* Draw model */
    seel_model_draw(model, shader);
}

void seel_renderer_set_clear_color(struct Renderer *renderer, vec3 color)
{
    glm_vec3_copy(color, renderer->clear_color);
}

void seel_renderer_set_active_shader(struct Renderer *renderer, struct Shader *shader)
{
    renderer->active_shader = shader;
}

#endif /* RENDERER_H */