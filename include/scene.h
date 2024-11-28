#ifndef SCENE_H
#define SCENE_H

#include "model.h"
#include "animator.h"
#include "renderer.h"

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
void seel_scene_add_model(struct Scene *scene, const char *name, struct Model *model, mat4 transform);
void seel_scene_update(struct Scene *scene, float delta_time);
void seel_scene_render(struct Scene *scene, struct Renderer *renderer);
void seel_scene_cleanup(struct Scene *scene);

void seel_scene_init(struct Scene *scene)
{
    scene->root_nodes = NULL;
    scene->num_root_nodes = 0;
}

void seel_scene_add_model(struct Scene *scene, const char *name, struct Model *model, mat4 transform)
{
    scene->root_nodes = realloc(scene->root_nodes, sizeof(struct SceneNode) * (scene->num_root_nodes + 1));
    struct SceneNode *node = &scene->root_nodes[scene->num_root_nodes++];

    strcpy(node->name, name);
    node->model = model;
    seel_animator_create(&node->animator);
    if (model->animated)
    {
        char temp[256];
        strcpy(temp, model->directory);
        node->animation = seel_animation_create(strcat(temp, model->name), model);
        seel_play_animation(&node->animator, &node->animation, true);
    }
    glm_mat4_copy(transform, node->transform);
    node->children = NULL;
    node->num_children = 0;
}

void seel_scene_update(struct Scene *scene, float delta_time)
{
    for (unsigned int i = 0; i < scene->num_root_nodes; ++i)
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

        // mat4 global_transform = GLM_MAT4_IDENTITY_INIT;
        // glm_mat4_copy(node->transform, global_transform);

        seel_renderer_draw_scene_node(renderer, node->model, &node->animator, node->transform);
    }
}

#endif /* SCENE_H */