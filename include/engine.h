#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include "window.h"
#include "camera.h"
#include "input.h"
#include "renderer.h"
#include "asset_manager.h"
#include "text.h"
#include "time.h"
#include "scene.h"
#include "particle.h"
#include "config.h"
#include "ui.h"

struct Engine
{
    struct EngineConfig config;
    GLFWwindow *window;
    struct Renderer renderer;
    struct Camera camera;
    struct Input input;
    struct AssetManager asset_manager;
    struct TimeManager time_manager;
    struct UIManager ui_manager;
    struct Scene scene;
    struct ParticleEmitter particle_emitter;
};

bool seel_engine_init(struct Engine *e);
void seel_engine_update(struct Engine *e);
void seel_engine_cleanup(struct Engine *e);

bool seel_engine_init(struct Engine *e)
{
    seel_config_init_defaults(&e->config);

    e->window = seel_create_window(e->config.window.width, e->config.window.height, e->config.window.title);
    if (!e->window)
        return false;

    seel_texture_init_stb();
    seel_freetype_init();

    seel_ui_init(&e->ui_manager, e->window);

    e->camera = seel_camera_create(&e->config.camera);

    seel_input_init(&e->input, e->window, &e->camera);

    e->asset_manager = seel_asset_manager_create();

    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_SHADER, "default", "../shaders/default.vert", "../shaders/default.frag"))
        return -1;
    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_SHADER, "lights", "../shaders/lights.vert", "../shaders/lights.frag"))
        return -1;
    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_SHADER, "text", "../shaders/text.vert", "../shaders/text.frag"))
        return -1;

    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_SHADER, "billboard", "../shaders/billboard.vert", "../shaders/billboard.frag"))
        return -1;

    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_SHADER, "billboardText", "../shaders/billboard.vert", "../shaders/text.frag"))
        return -1;

    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_SHADER, "particle", "../shaders/particle.vert", "../shaders/particle.frag"))
        return -1;

    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_MODEL, "vampire", "../assets/models/vampire/dancing_vampire.dae"))
        return -1;

    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_TEXTURE, "doge", "../assets/doge.png"))
        return -1;

    seel_scene_init(&e->scene);

    seel_scene_add_model(&e->scene, &e->asset_manager, "vampire1", "vampire", (vec3){0.01f, 0.01f, 0.01f}, (vec3){0.0f, 0.0f, 0.0f});
    seel_scene_add_model(&e->scene, &e->asset_manager, "vampire2", "vampire", (vec3){0.01f, 0.01f, 0.01f}, (vec3){300.0f, 0.0f, 0.0f});
    seel_scene_add_model(&e->scene, &e->asset_manager, "vampire3", "vampire", (vec3){0.01f, 0.01f, 0.01f}, (vec3){600.0f, 0.0f, 0.0f});
    seel_scene_add_model(&e->scene, &e->asset_manager, "vampire4", "vampire", (vec3){0.01f, 0.01f, 0.01f}, (vec3){900.0f, 0.0f, 0.0f});

    seel_renderer_init(&e->renderer, &e->config.renderer, &e->camera);
    seel_renderer_set_active_shader(&e->renderer, (struct Shader *)SEEL_ASSET_MANAGER_GET(&e->asset_manager, SHADER, "default"));

    e->particle_emitter = seel_particle_emitter_create((struct Shader *)SEEL_ASSET_MANAGER_GET(&e->asset_manager, SHADER, "particle"),
                                                       (struct Texture *)SEEL_ASSET_MANAGER_GET(&e->asset_manager, TEXTURE, "doge"), 1000, (vec3){0.0f, 0.0f, 0.0f});

    seel_time_init(&e->time_manager);

    return true;
}

void seel_engine_update(struct Engine *e)
{
    seel_time_update(&e->time_manager);

    seel_input_process(&e->input, e->window, e->time_manager.delta_time);

    seel_particle_emitter_update(&e->particle_emitter, e->time_manager.delta_time);

    GLint previous_program, previous_vao, previous_blend_src, previous_blend_dst;
    GLint previous_blend_equation, previous_active_texture, previous_viewport[4];
    GLboolean previous_blend_enabled, previous_depth_test, previous_cull_face;

    glGetIntegerv(GL_CURRENT_PROGRAM, &previous_program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_vao);
    glGetIntegerv(GL_BLEND_SRC_RGB, &previous_blend_src);
    glGetIntegerv(GL_BLEND_DST_RGB, &previous_blend_dst);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &previous_blend_equation);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &previous_active_texture);
    glGetIntegerv(GL_VIEWPORT, previous_viewport);

    previous_blend_enabled = glIsEnabled(GL_BLEND);
    previous_depth_test = glIsEnabled(GL_DEPTH_TEST);
    previous_cull_face = glIsEnabled(GL_CULL_FACE);

    seel_ui_begin_frame();
    if (nk_begin(e->ui_manager.ctx, "Demo", nk_rect(50, 50, 230, 250),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
                     NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
    {
        enum
        {
            EASY,
            HARD
        };
        static int op = EASY;
        static int property = 20;
        nk_layout_row_static(e->ui_manager.ctx, 30, 80, 1);
        if (nk_button_label(e->ui_manager.ctx, "button"))
            fprintf(stdout, "button pressed\n");
    }
    nk_end(e->ui_manager.ctx);
    seel_ui_end_frame();

    if (previous_blend_enabled)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

    glBlendFunc(previous_blend_src, previous_blend_dst);
    glBlendEquation(previous_blend_equation);

    if (previous_depth_test)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    if (previous_cull_face)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    glUseProgram(previous_program);
    glBindVertexArray(previous_vao);
    glActiveTexture(previous_active_texture);
    glViewport(previous_viewport[0], previous_viewport[1], previous_viewport[2], previous_viewport[3]);

    seel_renderer_begin_frame(&e->renderer);

    seel_scene_update(&e->scene, e->time_manager.delta_time);

    seel_scene_render(&e->scene, &e->renderer);

    seel_shader_set_int(e->renderer.active_shader, "enableDirectionalLight", 1);
    seel_shader_set_int(e->renderer.active_shader, "debugLighting", 0);
    seel_shader_set_float(e->renderer.active_shader, "material.shininess", 32.0f);
    seel_shader_set_int(e->renderer.active_shader, "material.diffuse1", 0);
    seel_shader_set_int(e->renderer.active_shader, "material.specular1", 1);
    seel_shader_set_vec3(e->renderer.active_shader, "dirLight.direction", (vec3){0.0f, -10.0f, 0.0f});
    seel_shader_set_vec3(e->renderer.active_shader, "dirLight.ambient", (vec3){0.1f, 0.1f, 0.1f});
    seel_shader_set_vec3(e->renderer.active_shader, "dirLight.diffuse", (vec3){1.0f, 1.0f, 1.0f});
    seel_shader_set_vec3(e->renderer.active_shader, "dirLight.specular", (vec3){1.0f, 1.0f, 1.0f});

    seel_particle_emitter_render(&e->particle_emitter, &e->camera);

    seel_renderer_draw_billboard((struct Shader *)SEEL_ASSET_MANAGER_GET(&e->asset_manager, SHADER, "billboard"),
                                 (struct Texture *)SEEL_ASSET_MANAGER_GET(&e->asset_manager, TEXTURE, "doge"),
                                 (vec3){0.0f}, 1.0f, (vec3){1.0f, 1.0f, 1.0f}, &e->camera);

    seel_render_text_billboard((struct Shader *)SEEL_ASSET_MANAGER_GET(&e->asset_manager, SHADER, "billboardText"), "Jamestiago", (vec3){1.0f, 2.0f, 3.0f}, 0.01f, (vec3){1.0f, 0.0f, 1.0f}, &e->camera);

    seel_render_text((struct Shader *)SEEL_ASSET_MANAGER_GET(&e->asset_manager, SHADER, "text"),
                     e->config.window.title, 10.0f, e->renderer.height - 35.0f, 0.5f, (vec3){1.0f, 1.0f, 1.0f}, e->renderer.width, e->renderer.height);
    char fps_text[32];
    sprintf(fps_text, "Framerate: %.f", e->time_manager.frame_rate);
    seel_render_text((struct Shader *)SEEL_ASSET_MANAGER_GET(&e->asset_manager, SHADER, "text"), fps_text, 10.0f, e->renderer.height - 60.0f, 0.5f, (vec3){1.0f, 1.0f, 1.0f}, e->renderer.width, e->renderer.height);

    seel_renderer_end_frame();
}

void seel_engine_cleanup(struct Engine *e)
{
    seel_asset_manager_cleanup(&e->asset_manager);
    seel_ui_cleanup();
    seel_particle_emitter_destroy(&e->particle_emitter);
    seel_destroy_window(e->window);
}

#endif /* ENGINE_H */