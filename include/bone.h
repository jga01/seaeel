#ifndef BONE_H
#define BONE_H

#include <string.h>

#include "cglm/cglm.h"
#include "assimp/scene.h"

#define MAX_BONE_NAME_LEN 64
#define MAX_BONE_INFLUENCE 4

struct KeyPosition
{
    vec3 position;
    float time_stamp;
};

struct KeyRotation
{
    vec4 rotation;
    float time_stamp;
};

struct KeyScale
{
    vec3 scale;
    float time_stamp;
};

struct BoneInfo
{
    char name[MAX_BONE_NAME_LEN];
    int id;
    mat4 offset;
};

struct Bone
{
    struct KeyPosition *positions;
    struct KeyRotation *rotations;
    struct KeyScale *scalings;
    int num_positions;
    int num_rotations;
    int num_scalings;
    mat4 transform;
    char name[MAX_BONE_NAME_LEN];
    int id;
};

struct Bone seel_bone_create(const char *name, int id, const struct aiNodeAnim *channel)
{
    struct Bone bone = {
        .transform = GLM_MAT4_IDENTITY_INIT,
        .id = id,
    };
    strcpy(bone.name, name);

    bone.num_positions = channel->mNumPositionKeys;
    bone.positions = (struct KeyPosition *)malloc(sizeof(struct KeyPosition) * bone.num_positions);
    if (!bone.positions)
    {
        fprintf(stderr, "Failed to allocate memory for bone positions!\n");
        return bone;
    }

    unsigned int position_index;
    for (position_index = 0; position_index < bone.num_positions; position_index++)
    {
        struct aiVector3D ai_position = channel->mPositionKeys[position_index].mValue;
        float time_stamp = channel->mPositionKeys[position_index].mTime;
        struct KeyPosition data;
        data.position[0] = ai_position.x;
        data.position[1] = ai_position.y;
        data.position[2] = ai_position.z;
        data.time_stamp = time_stamp;
        bone.positions[position_index] = data;
    }

    bone.num_rotations = channel->mNumRotationKeys;
    bone.rotations = (struct KeyRotation *)malloc(sizeof(struct KeyRotation) * bone.num_rotations);
    if (!bone.rotations)
    {
        fprintf(stderr, "Failed to allocate memory for bone rotations!\n");
        return bone;
    }

    unsigned int rotation_index = 0;
    for (rotation_index = 0; rotation_index < bone.num_rotations; rotation_index++)
    {
        struct aiQuaternion ai_rotation = channel->mRotationKeys[rotation_index].mValue;
        float time_stamp = channel->mRotationKeys[rotation_index].mTime;
        struct KeyRotation data;
        data.rotation[0] = ai_rotation.x;
        data.rotation[1] = ai_rotation.y;
        data.rotation[2] = ai_rotation.z;
        data.rotation[3] = ai_rotation.w;
        data.time_stamp = time_stamp;
        bone.rotations[rotation_index] = data;
    }

    bone.num_scalings = channel->mNumScalingKeys;
    bone.scalings = (struct KeyScale *)malloc(sizeof(struct KeyScale) * bone.num_scalings);
    if (!bone.scalings)
    {
        fprintf(stderr, "Failed to allocate memory for bone scalings!\n");
        return bone;
    }

    unsigned int scaling_index;
    for (scaling_index = 0; scaling_index < bone.num_scalings; scaling_index++)
    {
        struct aiVector3D ai_scaling = channel->mScalingKeys[scaling_index].mValue;
        float time_stamp = channel->mScalingKeys[scaling_index].mTime;
        struct KeyScale data;
        data.scale[0] = ai_scaling.x;
        data.scale[1] = ai_scaling.y;
        data.scale[2] = ai_scaling.z;
        data.time_stamp = time_stamp;
        bone.scalings[scaling_index] = data;
    }

    return bone;
}

unsigned int seel_get_position_index(struct Bone *bone, float animation_time)
{
    if (bone->num_positions <= 1)
        return 0;

    unsigned int index;
    for (index = 0; index < bone->num_positions - 1; index++)
    {
        if (animation_time < bone->positions[index + 1].time_stamp)
            return index;
    }

    return bone->num_positions - 2;
}

unsigned int seel_get_rotation_index(struct Bone *bone, float animation_time)
{
    if (bone->num_rotations <= 1)
        return 0;

    unsigned int index;
    for (index = 0; index < bone->num_rotations - 1; index++)
    {
        if (animation_time < bone->rotations[index + 1].time_stamp)
            return index;
    }
    return bone->num_rotations - 2;
}

unsigned int seel_get_scale_index(struct Bone *bone, float animation_time)
{
    if (bone->num_scalings <= 1)
        return 0;

    for (int index = 0; index < bone->num_scalings - 1; index++)
    {
        if (animation_time < bone->scalings[index + 1].time_stamp)
            return index;
    }

    return bone->num_scalings - 2;
}

float seel_get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time)
{
    float scale_factor = 0.0f;
    float mid_way_length = animation_time - last_time_stamp;
    float frames_diff = next_time_stamp - last_time_stamp;
    scale_factor = mid_way_length / frames_diff;
    return scale_factor;
}

void seel_interpolate_position(float animation_time, struct KeyPosition from, struct KeyPosition to, vec4 *dest)
{
    float scale_factor = seel_get_scale_factor(from.time_stamp, to.time_stamp, animation_time);
    vec3 final_position;
    glm_vec3_mix(from.position, to.position, scale_factor, final_position);
    glm_translate_to(GLM_MAT4_IDENTITY, final_position, dest);
}

void seel_interpolate_rotation(float animation_time, struct KeyRotation from, struct KeyRotation to, vec4 *dest)
{
    float scale_factor = seel_get_scale_factor(from.time_stamp, to.time_stamp, animation_time);
    vec4 final_rotation;
    glm_quat_slerp(from.rotation, to.rotation, scale_factor, final_rotation);
    glm_vec4_normalize(final_rotation);
    glm_quat_mat4(final_rotation, dest);
}

void seel_interpolate_scaling(float animation_time, struct KeyScale from, struct KeyScale to, vec4 *dest)
{
    float scale_factor = seel_get_scale_factor(from.time_stamp, to.time_stamp, animation_time);
    vec3 final_scale;
    glm_vec3_mix(from.scale, to.scale, scale_factor, final_scale);
    glm_scale_to(GLM_MAT4_IDENTITY, final_scale, dest);
}

void seel_bone_update(struct Bone *bone, float animation_time)
{
    unsigned int pos_index = seel_get_position_index(bone, animation_time);
    mat4 translation;
    if (bone->num_positions)
    {
        glm_translate_to(GLM_MAT4_IDENTITY, bone->positions[0].position, translation);
    }
    else
        seel_interpolate_position(animation_time, bone->positions[pos_index], bone->positions[pos_index + 1], translation);

    unsigned int rot_index = seel_get_rotation_index(bone, animation_time);
    mat4 rotation;
    if (bone->num_rotations == 1)
    {
        vec4 norm;
        glm_normalize_to(bone->rotations[0].rotation, norm);
        glm_quat_mat4(norm, rotation);
    }
    else
        seel_interpolate_rotation(animation_time, bone->rotations[rot_index], bone->rotations[rot_index + 1], rotation);

    unsigned int scl_index = seel_get_scale_index(bone, animation_time);
    mat4 scale;
    if (bone->num_scalings == 1)
        glm_scale_to(GLM_MAT4_IDENTITY, bone->scalings[0].scale, scale);
    else
        seel_interpolate_scaling(animation_time, bone->scalings[scl_index], bone->scalings[scl_index + 1], scale);

    glm_mat4_mulN((mat4 *[]){&translation, &rotation, &scale}, 3, bone->transform);
}

#endif /* BONE_H */