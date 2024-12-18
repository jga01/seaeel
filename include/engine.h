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
#include "config.h"

struct Engine
{
    struct EngineConfig config;
    GLFWwindow *window;
    struct Renderer renderer;
    struct Camera camera;
    struct Input input;
    struct AssetManager asset_manager;
    struct TimeManager time_manager;
    struct Scene scene;
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

    e->camera = seel_camera_create(&e->config.camera);

    seel_input_init(&e->input, e->window, &e->camera);

    e->asset_manager = seel_asset_manager_create();

    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_SHADER, "default", "../shaders/default.vert", "../shaders/default.frag"))
        return -1;
    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_SHADER, "lights", "../shaders/lights.vert", "../shaders/lights.frag"))
        return -1;
    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_SHADER, "text", "../shaders/text.vert", "../shaders/text.frag"))
        return -1;

    if (!SEEL_ASSET_MANAGER_LOAD(&e->asset_manager, ASSET_MODEL, "vampire", "../assets/models/vampire/dancing_vampire.dae"))
        return -1;

    seel_scene_init(&e->scene);

    seel_scene_add_model(&e->scene, &e->asset_manager, "vampire1", "vampire", (vec3){0.01f, 0.01f, 0.01f}, (vec3){0.0f, 0.0f, 0.0f});
    seel_scene_add_model(&e->scene, &e->asset_manager, "vampire2", "vampire", (vec3){0.01f, 0.01f, 0.01f}, (vec3){300.0f, 0.0f, 0.0f});
    seel_scene_add_model(&e->scene, &e->asset_manager, "vampire3", "vampire", (vec3){0.01f, 0.01f, 0.01f}, (vec3){600.0f, 0.0f, 0.0f});
    seel_scene_add_model(&e->scene, &e->asset_manager, "vampire4", "vampire", (vec3){0.01f, 0.01f, 0.01f}, (vec3){900.0f, 0.0f, 0.0f});
    seel_scene_add_model(&e->scene, &e->asset_manager, "vampire5", "vampire", (vec3){0.01f, 0.01f, 0.01f}, (vec3){1200.0f, 0.0f, 0.0f});

    seel_renderer_init(&e->renderer, &e->config.renderer, &e->camera);
    seel_renderer_set_active_shader(&e->renderer, (struct Shader *)SEEL_ASSET_MANAGER_GET(&e->asset_manager, SHADER, "default"));

    seel_time_init(&e->time_manager);

    return true;
}

void seel_engine_update(struct Engine *e)
{
    seel_time_update(&e->time_manager);

    seel_input_process(&e->input, e->window, e->time_manager.delta_time);

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
    seel_shader_set_vec3(e->renderer.active_shader, "dirLight.specular", (vec3){1.0f, 1.0f, 1.0f});;

    seel_render_text((struct Shader *)SEEL_ASSET_MANAGER_GET(&e->asset_manager, SHADER, "text"),
                     "Seaeel Engine 0.1v", 10.0f, e->renderer.height - 35.0f, 0.5f, (vec3){1.0f, 1.0f, 1.0f}, e->renderer.width, e->renderer.height);
    char fps_text[32];
    sprintf(fps_text, "Framerate: %.f", e->time_manager.frame_rate);
    seel_render_text((struct Shader *)SEEL_ASSET_MANAGER_GET(&e->asset_manager, SHADER, "text"), fps_text, 10.0f, e->renderer.height - 60.0f, 0.5f, (vec3){1.0f, 1.0f, 1.0f}, e->renderer.width, e->renderer.height);

    seel_renderer_end_frame();
}

void seel_engine_cleanup(struct Engine *e)
{
    seel_asset_manager_cleanup(&e->asset_manager);
    seel_destroy_window(e->window);
}

#endif /* ENGINE_H */