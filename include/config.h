#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#include "cglm/cglm.h"
#include "camera.h"

struct WindowConfig
{
    unsigned int width;
    unsigned int height;
    char title[256];
};

struct RendererConfig
{
    unsigned int width;
    unsigned int height;
    vec3 clear_color;
    unsigned int max_vertex_buffer;
    unsigned int max_element_buffer;
    bool enable_debug_output;
    bool enable_depth_test;
    bool enable_blending;
};

struct CameraConfig
{
    float yaw;
    float pitch;
    float fov;
    float near_clip;
    float far_clip;
    float movement_speed;
    float mouse_sensitivity;
    float zoom;
    enum CameraMode mode;
    vec3 initial_position;
    bool invert_y;
};

struct InputConfig
{
};

struct EngineConfig
{
    struct WindowConfig window;
    struct RendererConfig renderer;
    struct CameraConfig camera;
    struct InputConfig input;
    const char *asset_path;
    bool enable_debug;
};

void seel_config_init_defaults(struct EngineConfig *engine_config);
bool seel_config_load_from_file(struct EngineConfig *engine_config, const char *filename);

void seel_config_init_defaults(struct EngineConfig *engine_config)
{
    engine_config->window.width = 1280;
    engine_config->window.height = 720;
    strcpy(engine_config->window.title, "Seaeel Engine v0.1");

    glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, engine_config->renderer.clear_color);
    engine_config->renderer.max_element_buffer = 512 * 1024;
    engine_config->renderer.max_vertex_buffer = 128 * 1024;
    engine_config->renderer.width = engine_config->window.width;
    engine_config->renderer.height = engine_config->window.height;
    engine_config->renderer.enable_debug_output = true;
    engine_config->renderer.enable_depth_test = true;
    engine_config->renderer.enable_blending = true;

    engine_config->camera.fov = 45.0f;
    engine_config->camera.near_clip = 0.1f;
    engine_config->camera.far_clip = 100.0f;
    engine_config->camera.movement_speed = 2.5f;
    engine_config->camera.mouse_sensitivity = 0.1f;
    engine_config->camera.invert_y = false;
    engine_config->camera.zoom = 45.0f;
    glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, engine_config->camera.initial_position);
    engine_config->camera.mode = CAMERA_MODE_FREE;
    engine_config->camera.yaw = -90.0f;
    engine_config->camera.pitch = 0.0f;
};

#endif /* CONFIG_H */