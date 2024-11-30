#ifndef SCENE_H
#define SCENE_H

#include "model.h"
#include "animator.h"
#include "renderer.h"
#include "asset_manager.h"

#define MAX_NODE_NAME_LEN 512

struct SceneNode
{
    char name[MAX_NODE_NAME_LEN];
    struct Model *model;
    struct Animation animation;
    struct Animator animator;
    mat4 transform;
    struct SceneNode *children;
    unsigned int num_children;
};

struct Scene
{
    struct SceneNode *root_nodes;
    unsigned int num_root_nodes;
};

void seel_scene_init(struct Scene *scene);
void seel_scene_add_model(struct Scene *scene, struct AssetManager *asset_manager, const char *model_id, const char *asset_name, vec3 scale, vec3 translate);
void seel_scene_update(struct Scene *scene, float delta_time);
void seel_scene_render(struct Scene *scene, struct Renderer *renderer);
void seel_scene_cleanup(struct Scene *scene);

void seel_scene_init(struct Scene *scene)
{
    scene->root_nodes = NULL;
    scene->num_root_nodes = 0;
}

void seel_scene_add_model(struct Scene *scene, struct AssetManager *asset_manager, const char *model_id, const char *asset_name, vec3 scale, vec3 translate)
{
    mat4 model_matrix = GLM_MAT4_IDENTITY_INIT;

    // Apply scaling
    glm_scale(model_matrix, scale);

    // Apply translation
    glm_translate(model_matrix, translate);

    // Retrieve the model and add it to the scene
    struct Model *model = (struct Model *)SEEL_ASSET_MANAGER_GET(asset_manager, MODEL, asset_name);
    if (model)
    {
        struct SceneNode *old_nodes = scene->root_nodes;

        scene->root_nodes = realloc(scene->root_nodes, sizeof(struct SceneNode) * (scene->num_root_nodes + 1));
        if (!scene->root_nodes)
        {
            /* Handle allocation failure */
            fprintf(stderr, "Failed to reallocate memory for scene nodes.\n");
            exit(EXIT_FAILURE);
        }

        /* Check if realloc moved the memory */
        if (scene->root_nodes != old_nodes)
        {
            /* Fix all internal pointers within existing nodes */
            for (unsigned int i = 0; i < scene->num_root_nodes; i++)
            {
                struct SceneNode *node = &scene->root_nodes[i];
                if (node->animator.current_animation)
                {
                    node->animator.current_animation = (struct Animation *)((char *)node->animator.current_animation + ((char *)scene->root_nodes - (char *)old_nodes));
                }
                /* Repeat this for other pointers if needed */
            }
        }

        struct SceneNode *node = &scene->root_nodes[scene->num_root_nodes++];
        memset(node, 0, sizeof(struct SceneNode));

        strcpy(node->name, model_id);
        node->model = model;
        seel_animator_create(&node->animator);
        if (model->animated)
        {
            char temp[256];
            strcpy(temp, model->directory);
            snprintf(temp, sizeof(temp), "%s%s", model->directory, model->name);
            node->animation = seel_animation_create(temp, model);
            seel_play_animation(&node->animator, &node->animation, true);
        }
        glm_mat4_copy(model_matrix, node->transform);
        node->children = NULL;
        node->num_children = 0;
    }
    else
    {
        printf("Error: Model asset '%s' not found.\n", asset_name);
    }
}

void seel_scene_update(struct Scene *scene, float delta_time)
{
    for (unsigned int i = 0; i < scene->num_root_nodes; i++)
    {
        struct SceneNode *node = &scene->root_nodes[i];
        if (node->animator.current_animation)
        {
            seel_update_animation(&node->animator, delta_time);
        }
    }
}

void seel_scene_render(struct Scene *scene, struct Renderer *renderer)
{
    for (unsigned int i = 0; i < scene->num_root_nodes; ++i)
    {
        struct SceneNode *node = &scene->root_nodes[i];
        seel_renderer_draw_scene_node(renderer, node->model, &node->animator, node->transform);
    }
}

#endif /* SCENE_H */