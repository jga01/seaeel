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
#include "animator.h"

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
    glfwGetFramebufferSize(glfwGetCurrentContext(), &renderer->width, &renderer->height);
    glViewport(0, 0, renderer->width, renderer->height);
    glClearColor(renderer->clear_color[0], renderer->clear_color[1], renderer->clear_color[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void seel_renderer_end_frame(void)
{
    glfwSwapBuffers(glfwGetCurrentContext());
    glfwPollEvents();
}

void seel_renderer_draw_scene_node(struct Renderer *renderer, struct Model *model, struct Animator *animator, mat4 model_matrix)
{
    if (!renderer->active_shader)
    {
        fprintf(stderr, "No active shader set for renderer.\n");
        return;
    }
    struct Shader *shader = renderer->active_shader;
    seel_shader_use(shader);

    /* Set matrices */
    seel_shader_set_mat4(shader, "model", &model_matrix[0][0]);

    mat4 view;
    seel_camera_get_view_matrix(renderer->camera, view);
    seel_shader_set_mat4(shader, "view", &view[0][0]);

    mat4 projection;
    glm_perspective(glm_rad(renderer->camera->zoom),
                    (float)renderer->width / (float)renderer->height,
                    renderer->camera->near_clip, renderer->camera->far_clip, projection);
    seel_shader_set_mat4(shader, "projection", &projection[0][0]);

    mat4 normal_matrix = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_inv(model_matrix, normal_matrix);
    glm_mat4_transpose(normal_matrix);
    seel_shader_set_mat4(shader, "normalMatrix", &normal_matrix[0][0]);

    /* Set camera position */
    seel_shader_set_vec3(shader, "viewPos", renderer->camera->position);

    seel_shader_set_int(shader, "animate", model->animated);

    if (model->animated && animator->final_bone_matrices)
    {
        for (int i = 0; i < animator->num_matrices; i++)
        {
            char uniform_name[32];
            snprintf(uniform_name, sizeof(uniform_name), "boneMatrices[%d]", i);
            seel_shader_set_mat4(shader, uniform_name, &animator->final_bone_matrices[i][0][0]);
        }
    }

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