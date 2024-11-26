#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H

#include <stdbool.h>
#include "cglm/cglm.h"

struct Engine;
struct AssetManager;
struct Renderer;
struct Camera;
struct Input;
struct TimeManager;
// struct EventSystem;

// typedef void (*EventCallback)(void *data);
// typedef void (*UpdateCallback)(struct Engine *engine, float delta_time);
// typedef void (*RenderCallback)(struct Engine *engine, float delta_time);

// enum EventType
// {
//     EVENT_WINDOW_RESIZE,
//     EVENT_KEY_PRESS,
//     EVENT_KEY_RELEASE,
//     EVENT_MOUSE_MOVE,
//     EVENT_MOUSE_SCROLL,
//     EVENT_ASSET_LOADED,
//     EVENT_CONFIG_CHANGED,
//     EVENT_COUNT
// };

enum CameraMode
{
    CAMERA_MODE_FPS,
    CAMERA_MODE_ORBITAL,
    CAMERA_MODE_FREE
};

#endif /* ENGINE_TYPES_H */