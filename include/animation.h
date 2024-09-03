#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdlib.h>

#include "cglm/cglm.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "bone.h"
#include "model.h"

struct assimp_node_data
{
    mat4 transformation;
    char name[64];
    int children_count;
    struct assimp_node_data *children;
};

typedef struct animation
{
    float duration;
    int ticks_per_second;
    bone bones[100];
    int num_bones;
    struct assimp_node_data root_node;
    bone_info *bone_info;
    int bone_info_size;
} animation;

void load_intermediate_bones(animation *a, const struct aiAnimation *animation, model *model)
{
    bone_info *bones_info = model->bone_info;
    // if (!a->bones)
    // {
    //     a->bones = (bone *)malloc(sizeof(bone));
    //     if (!a->bones)
    //     {
    //         fprintf(stderr, "Failed to allocate memory for bones!\n");
    //         return;
    //     }
    // }

    for (unsigned int i = 0; i < animation->mNumChannels; i++)
    {
        struct aiNodeAnim *channel = animation->mChannels[i];
        char *bone_name = channel->mNodeName.data;
        int bone_id = -1;

        for (unsigned int i = 0; i < model->bone_counter; i++)
        {
            if (!strcmp(bones_info[i].name, bone_name))
            {
                bone_id = i;
                break;
            }
        }

        if (bone_id == -1)
        {
            struct bone_info new_bone_info;
            strcpy(new_bone_info.name, bone_name);
            bones_info = (bone_info *)realloc(bones_info, sizeof(bone_info) * (model->bone_counter + 1));
            if (!bones_info)
            {
                fprintf(stderr, "Failed to reallocate memory for bone info!\n");
                return;
            }
            bones_info[model->bone_counter++] = new_bone_info;
            bone_id = model->bone_counter - 1;
        }

        // a->bones = (bone *)realloc(a->bones, sizeof(bone) * (a->num_bones + 1));
        // if (!a->bones)
        // {
        //     fprintf(stderr, "Failed to reallocate memory for a->bones!\n");
        //     return;
        // }

        a->bones[a->num_bones++] = bone_create(bone_name, bone_id, channel);
    }

    a->bone_info = bones_info;
    a->bone_info_size = model->bone_counter;
}

void generate_bone_tree(struct assimp_node_data *parent, const struct aiNode *src)
{
    assert(src);

    strcpy(parent->name, src->mName.data);
    ai_matrix_to_glm_mat4(&src->mTransformation, parent->transformation);
    parent->children_count = src->mNumChildren;

    parent->children = (struct assimp_node_data *)malloc(sizeof(struct assimp_node_data) * parent->children_count);
    if (!parent->children)
    {
        fprintf(stderr, "Failed to allocate memory for parent children bones!\n");
        return;
    }

    for (unsigned int i = 0; i < src->mNumChildren; i++)
    {
        generate_bone_tree(&parent->children[i], src->mChildren[i]);
    }
}

void print_bone_tree(const struct assimp_node_data *node, int depth);

animation animation_create(const char *path, model *m)
{
    animation a = {0};

    const struct aiScene *scene = aiImportFile(path, aiProcess_Triangulate);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        fprintf(stderr, "Failed in loading animation!\n\n%s\n", aiGetErrorString());
        return a;
    }

    if (scene->mNumAnimations == 0)
        return a;

    struct aiAnimation *animation = scene->mAnimations[0];
    a.duration = (float)animation->mDuration;
    a.ticks_per_second = (float)animation->mTicksPerSecond;
    generate_bone_tree(&a.root_node, scene->mRootNode);
    glm_mat4_copy(GLM_MAT4_IDENTITY, a.root_node.transformation);
    load_intermediate_bones(&a, animation, m);

    // print_bone_tree(&a.root_node, 0);

    return a;
}

bone find_bone(animation *a, const char *name)
{
    bone b = {0};
    b.id = -1;

    for (unsigned int i = 0; i < a->num_bones; i++)
    {
        if (!strcmp(a->bones[i].name, name))
        {
            return a->bones[i];
        }
    }
    return b;
}

void print_bone_tree(const struct assimp_node_data *node, int depth)
{
    if (!node)
        return;

    // Print the current node's name and transformation matrix
    for (int i = 0; i < depth; i++)
        printf("  ");  // Indentation for visual hierarchy
    printf("Node Name: %s\n", node->name);
    printf("Transformation Matrix:\n");
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("%6.2f ", node->transformation[i][j]);
        }
        printf("\n");
    }

    // Recursively print each child node
    for (int i = 0; i < node->children_count; i++)
    {
        print_bone_tree(&node->children[i], depth + 1);
    }
}

#endif /* ANIMATION_H */