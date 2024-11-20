#ifndef RESOURCE_H
#define RESOURCE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shader.h"
#include "texture.h"
#include "model.h"
#include "animation.h"

#define MAX_ASSET_NAME_LEN 128

enum AssetType
{
    ASSET_SHADER,
    ASSET_TEXTURE,
    ASSET_MODEL,
    ASSET_TYPE_COUNT
};

union AssetData
{
    struct Shader *shader;
    struct Texture *texture;
    struct Model *model;
    struct Animation *animation;
};

struct Asset
{
    char name[MAX_ASSET_NAME_LEN];
    union AssetData data;
    enum AssetType type;
};

struct AssetManager
{
    struct Asset *assets;
    unsigned int num_assets;
};

struct AssetManager seel_asset_manager_create(void);
void seel_asset_manager_cleanup(struct AssetManager *manager);
bool seel_asset_manager_load(struct AssetManager *manager, enum AssetType type, const char *name, ...);
void *seel_asset_manager_get(const struct AssetManager *manager, enum AssetType type, const char *name);

#define SEEL_ASSET_MANAGER_LOAD(manager, type, name, ...) \
    seel_asset_manager_load(manager, type, name, __VA_ARGS__)

#define SEEL_ASSET_MANAGER_GET(manager, type, name) \
    (seel_asset_manager_get(manager, ASSET_##type, name))

struct AssetManager seel_asset_manager_create(void)
{
    return (struct AssetManager){.assets = NULL, .num_assets = 0};
}

void seel_asset_manager_cleanup(struct AssetManager *manager)
{
    for (unsigned int i = 0; i < manager->num_assets; i++)
    {
        struct Asset *asset = &manager->assets[i];
        switch (asset->type)
        {
        case ASSET_SHADER:
            free(asset->data.shader);
            break;
        case ASSET_TEXTURE:
            free(asset->data.texture);
            break;
        case ASSET_MODEL:
            free(asset->data.model);
            break;
        default:
            break;
        }
    }
    free(manager->assets);
    manager->assets = NULL;
    manager->num_assets = 0;
}

bool seel_asset_manager_load(struct AssetManager *manager, enum AssetType type, const char *name, ...)
{
    struct Asset asset;
    strncpy(asset.name, name, MAX_ASSET_NAME_LEN - 1);
    asset.name[MAX_ASSET_NAME_LEN - 1] = '\0';
    asset.type = type;

    va_list args;
    va_start(args, name);
    bool success = true;

    switch (type)
    {
    case ASSET_SHADER:
    {
        const char *vertex_path = va_arg(args, const char *);
        const char *fragment_path = va_arg(args, const char *);
        asset.data.shader = malloc(sizeof(struct Shader));
        if (asset.data.shader)
        {
            *asset.data.shader = seel_shader_create(vertex_path, fragment_path);
        }
        else
        {
            fprintf(stderr, "Failed to allocate memory for shader resource.\n");
            success = false;
        }
        break;
    }
    case ASSET_TEXTURE:
    {
        const char *path = va_arg(args, const char *);
        enum TextureType tex_type = va_arg(args, enum TextureType);
        asset.data.texture = malloc(sizeof(struct Texture));
        if (asset.data.texture)
        {
            *asset.data.texture = seel_texture_create(path, tex_type);
        }
        else
        {
            fprintf(stderr, "Failed to allocate memory for texture resource.\n");
            success = false;
        }
        break;
    }
    case ASSET_MODEL:
    {
        const char *path = va_arg(args, const char *);
        asset.data.model = malloc(sizeof(struct Model));
        if (asset.data.model)
        {
            *asset.data.model = seel_model_load(path);
        }
        else
        {
            fprintf(stderr, "Failed to allocate memory for model resource.\n");
            success = false;
        }
        break;
    }
    default:
        fprintf(stderr, "Unknown asset type.\n");
        success = false;
        break;
    }
    va_end(args);

    if (success)
    {
        manager->assets = realloc(manager->assets, (manager->num_assets + 1) * sizeof(struct Asset));
        manager->assets[manager->num_assets++] = asset;
    }

    return success;
}

void *seel_asset_manager_get(const struct AssetManager *manager, enum AssetType type, const char *name)
{
    for (unsigned int i = 0; i < manager->num_assets; i++)
    {
        if (manager->assets[i].type == type && strcmp(manager->assets[i].name, name) == 0)
        {
            switch (type)
            {
            case ASSET_SHADER:
                return manager->assets[i].data.shader;
            case ASSET_TEXTURE:
                return manager->assets[i].data.texture;
            case ASSET_MODEL:
                return manager->assets[i].data.model;
            default:
                return NULL;
            }
        }
    }
    return NULL;
}

#endif /* RESOURCE_H */