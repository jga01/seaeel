#ifndef RESOURCE_H
#define RESOURCE_H

#include <stdbool.h>
#include "shader.h"
#include "texture.h"
#include "model.h"
#include "animation.h"

#define MAX_ASSET_NAME_LEN  128

struct Asset
{
    char name[128];
    void *data;
    unsigned int size;
};

struct AssetManager
{
    struct Asset *assets;
    unsigned int num_assets;
};

bool seel_asset_manager_load_shader(struct AssetManager *manager, const char *name, const char *vertex_path, const char *fragment_path);
bool seel_asset_manager_load_texture(struct AssetManager *manager, const char *name, const char *path);
bool seel_asset_manager_load_model(struct AssetManager *manager, const char *name, const char *path);
bool seel_asset_manager_load_animation(struct AssetManager *manager, const char *name, const char *path, struct Model *model);

struct Shader *seel_asset_manager_get_shader(struct AssetManager *manager, const char *name);
struct Texture *seel_asset_manager_get_texture(struct AssetManager *manager, const char *name);
struct Model *seel_asset_manager_get_model(struct AssetManager *manager, const char *name);
struct Animation *seel_asset_manager_get_animation(struct AssetManager *manager, const char *name);

struct AssetManager seel_asset_manager_create(void)
{
    return (struct AssetManager){
        .assets = NULL,
        .num_assets = 0};
}

void seel_asset_manager_cleanup(struct AssetManager *manager)
{
    unsigned int i;
    for (i = 0; i < manager->num_assets; i++)
    {
    }
    free(manager->assets);
    manager->assets = NULL;
    manager->num_assets = 0;
}

bool seel_asset_manager_load_shader(struct AssetManager *manager, const char *name, const char *vertex_path, const char *fragment_path)
{
    struct Shader shader = seel_shader_create(vertex_path, fragment_path);

    struct Asset asset = {
        .name = strdup(name),
        .data = &shader,
        .size = sizeof(shader)};

    manager->assets = realloc(manager->assets, (manager->num_assets + 1) * sizeof(struct Asset));
    manager->assets[manager->num_assets++] = asset;
    return true;
}

struct Shader *seel_asset_manager_get_shader(struct AssetManager *manager, const char *name)
{
    unsigned int i;
    for (i = 0; i < manager->num_assets; i++)
    {
        if (strcmp(manager->assets[i].name, name) == 0)
        {
            return (struct Shader *)manager->assets[i].data;
        }
    }
    return NULL;
}

#endif /* RESOURCE_H */