#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdlib.h>

#include "cglm/cglm.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "bone.h"
#include "model.h"

#define MAX_NAME_LEN 64

struct AssimpNodeData
{
    mat4 transformation;
    char name[MAX_NAME_LEN];
    unsigned int children_count;
    struct AssimpNodeData *children;
};

struct Animation
{
    char name[MAX_NAME_LEN];
    float duration;
    int ticks_per_second;
    struct Bone bones[MAX_BONES];
    unsigned int num_bones;
    struct AssimpNodeData root_node;
    struct BoneInfo *bone_info;
    unsigned int bone_info_size;
};

void seel_load_intermediate_bones(struct Animation *animation, const struct aiAnimation *ai_animation, struct Model *model)
{
    struct BoneInfo *bones_info = model->bone_info;

    unsigned int i;
    for (i = 0; i < ai_animation->mNumChannels; i++)
    {
        struct aiNodeAnim *channel = ai_animation->mChannels[i];
        char *bone_name = channel->mNodeName.data;
        int bone_id = -1;

        unsigned int j;
        for (j = 0; j < model->bone_counter; j++)
        {
            if (!strcmp(bones_info[j].name, bone_name))
            {
                bone_id = j;
                break;
            }
        }

        if (bone_id == -1)
        {
            struct BoneInfo new_bone_info;
            strcpy(new_bone_info.name, bone_name);
            bones_info = (struct BoneInfo *)realloc(bones_info, sizeof(struct BoneInfo) * (model->bone_counter + 1));
            if (!bones_info)
            {
                fprintf(stderr, "Failed to reallocate memory for bone info!\n");
                return;
            }
            bones_info[model->bone_counter++] = new_bone_info;
            bone_id = model->bone_counter - 1;
        }

        animation->bones[animation->num_bones++] = seel_bone_create(bone_name, bone_id, channel);
    }

    animation->bone_info = bones_info;
    animation->bone_info_size = model->bone_counter;
}

void seel_generate_bone_tree(struct AssimpNodeData *parent, const struct aiNode *src)
{
    assert(src);

    strcpy(parent->name, src->mName.data);
    seel_ai_matrix_to_glm_mat4(&src->mTransformation, parent->transformation);
    parent->children_count = src->mNumChildren;

    parent->children = (struct AssimpNodeData *)malloc(sizeof(struct AssimpNodeData) * parent->children_count);
    if (!parent->children)
    {
        fprintf(stderr, "Failed to allocate memory for parent children bones!\n");
        return;
    }

    unsigned int i;
    for (i = 0; i < src->mNumChildren; i++)
    {
        seel_generate_bone_tree(&parent->children[i], src->mChildren[i]);
    }
}

struct Animation seel_animation_create(const char *path, struct Model *model)
{
    struct Animation animation = {0};
    memset(&animation, 0, sizeof(struct Animation));

    const struct aiScene *scene = aiImportFile(path, aiProcess_Triangulate);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        fprintf(stderr, "Failed in loading animation!\n\n%s\n", aiGetErrorString());
        return animation;
    }

    if (scene->mNumAnimations == 0)
        return animation;

    struct aiAnimation *ai_animation = scene->mAnimations[0];
    strcpy(animation.name, ai_animation->mName.data);
    animation.duration = (float)ai_animation->mDuration;
    animation.ticks_per_second = (float)ai_animation->mTicksPerSecond;
    seel_generate_bone_tree(&animation.root_node, scene->mRootNode);
    glm_mat4_copy(GLM_MAT4_IDENTITY, animation.root_node.transformation);
    seel_load_intermediate_bones(&animation, ai_animation, model);

    aiReleaseImport(scene);

    return animation;
}

struct Bone seel_find_bone(struct Animation *animation, const char *name)
{
    struct Bone bone = {0};
    bone.id = -1;

    unsigned int i;
    for (i = 0; i < animation->num_bones; i++)
    {
        if (!strcmp(animation->bones[i].name, name))
        {
            return animation->bones[i];
        }
    }
    return bone;
}

#endif /* ANIMATION_H */