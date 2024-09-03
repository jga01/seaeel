#ifndef BONE_H
#define BONE_H

#include <string.h>

#include "cglm/cglm.h"
#include "assimp/scene.h"

#define MAX_BONE_INFLUENCE 4

struct key_position
{
    vec3 position;
    float time_stamp;
};

struct key_rotation
{
    vec4 rotation;
    float time_stamp;
};

struct key_scaling
{
    vec3 scale;
    float time_stamp;
};

typedef struct bone_info
{
    char name[64];
    int id;
    mat4 offset;
} bone_info;

typedef struct bone
{
    struct key_position *positions;
    struct key_rotation *rotations;
    struct key_scaling *scalings;
    int num_positions;
    int num_rotations;
    int num_scalings;
    mat4 transform;
    char name[64];
    int id;
} bone;

bone bone_create(char *name, int id, const struct aiNodeAnim *channel)
{
    bone b = {
        .transform = GLM_MAT4_IDENTITY_INIT,
        .id = id,
    };
    strcpy(b.name, name);

    b.num_positions = channel->mNumPositionKeys;
    b.positions = (struct key_position *)malloc(sizeof(struct key_position) * b.num_positions);
    if (!b.positions)
    {
        fprintf(stderr, "Failed to allocate memory for bone positions!\n");
        return b;
    }

    for (int position_index = 0; position_index < b.num_positions; ++position_index)
    {
        struct aiVector3D ai_position = channel->mPositionKeys[position_index].mValue;
        float time_stamp = channel->mPositionKeys[position_index].mTime;
        struct key_position data;
        data.position[0] = ai_position.x;
        data.position[1] = ai_position.y;
        data.position[2] = ai_position.z;
        data.time_stamp = time_stamp;
        b.positions[position_index] = data;
    }

    b.num_rotations = channel->mNumRotationKeys;
    b.rotations = (struct key_rotation *)malloc(sizeof(struct key_rotation) * b.num_rotations);
    if (!b.rotations)
    {
        fprintf(stderr, "Failed to allocate memory for bone rotations!\n");
        return b;
    }

    for (int rotation_index = 0; rotation_index < b.num_rotations; ++rotation_index)
    {
        struct aiQuaternion ai_rotation = channel->mRotationKeys[rotation_index].mValue;
        float time_stamp = channel->mRotationKeys[rotation_index].mTime;
        struct key_rotation data;
        data.rotation[0] = ai_rotation.x;
        data.rotation[1] = ai_rotation.y;
        data.rotation[2] = ai_rotation.z;
        data.rotation[3] = ai_rotation.w;
        data.time_stamp = time_stamp;
        b.rotations[rotation_index] = data;
    }

    b.num_scalings = channel->mNumScalingKeys;
    b.scalings = (struct key_scaling *)malloc(sizeof(struct key_scaling) * b.num_scalings);
    if (!b.scalings)
    {
        fprintf(stderr, "Failed to allocate memory for bone scalings!\n");
        return b;
    }

    for (int scaling_index = 0; scaling_index < b.num_scalings; ++scaling_index)
    {
        struct aiVector3D ai_scaling = channel->mScalingKeys[scaling_index].mValue;
        float time_stamp = channel->mScalingKeys[scaling_index].mTime;
        struct key_scaling data;
        data.scale[0] = ai_scaling.x;
        data.scale[1] = ai_scaling.y;
        data.scale[2] = ai_scaling.z;
        data.time_stamp = time_stamp;
        b.scalings[scaling_index] = data;
    }

    return b;
}

int get_position_index(bone *b, float animation_time)
{
    if (b->num_positions <= 1)
        return 0;

    for (int index = 0; index < b->num_positions - 1; ++index)
    {
        if (animation_time < b->positions[index + 1].time_stamp)
            return index;
    }

    return b->num_positions - 2;
}

int get_rotation_index(bone *b, float animation_time)
{
    if (b->num_rotations <= 1)
        return 0;

    for (int index = 0; index < b->num_rotations - 1; ++index)
    {
        if (animation_time < b->rotations[index + 1].time_stamp)
            return index;
    }
    return b->num_rotations - 2;
}

int get_scale_index(bone *b, float animation_time)
{
    if (b->num_scalings <= 1)
        return 0;

    for (int index = 0; index < b->num_scalings - 1; ++index)
    {
        if (animation_time < b->scalings[index + 1].time_stamp)
            return index;
    }

    return b->num_scalings - 2;
}

float get_scale_factor(float last_time_stamp, float next_time_stamp, float animation_time)
{
    float scale_factor = 0.0f;
    float mid_way_length = animation_time - last_time_stamp;
    float frames_diff = next_time_stamp - last_time_stamp;
    scale_factor = mid_way_length / frames_diff;
    return scale_factor;
}

void interpolate_position(float animation_time, struct key_position from, struct key_position to, vec4 *dest)
{
    float scale_factor = get_scale_factor(from.time_stamp, to.time_stamp, animation_time);
    vec3 final_position;
    glm_vec3_mix(from.position, to.position, scale_factor, final_position);
    glm_translate_to(GLM_MAT4_IDENTITY, final_position, dest);
}

void interpolate_rotation(float animation_time, struct key_rotation from, struct key_rotation to, vec4 *dest)
{
    float scale_factor = get_scale_factor(from.time_stamp, to.time_stamp, animation_time);
    vec4 final_rotation;
    glm_quat_slerp(from.rotation, to.rotation, scale_factor, final_rotation);
    glm_vec4_normalize(final_rotation);
    glm_quat_mat4(final_rotation, dest); /* check if breaking */
}

void interpolate_scaling(float animation_time, struct key_scaling from, struct key_scaling to, vec4 *dest)
{
    float scale_factor = get_scale_factor(from.time_stamp, to.time_stamp, animation_time);
    vec3 final_scale;
    glm_vec3_mix(from.scale, to.scale, scale_factor, final_scale);
    glm_scale_to(GLM_MAT4_IDENTITY, final_scale, dest);
}

void update(bone *b, float animation_time)
{
    size_t pos_index = get_position_index(b, animation_time);
    mat4 translation;
    if (b->num_positions)
    {
        glm_translate_to(GLM_MAT4_IDENTITY, b->positions[0].position, translation);
    }
    else
        interpolate_position(animation_time, b->positions[pos_index], b->positions[pos_index + 1], translation);

    size_t rot_index = get_rotation_index(b, animation_time);
    mat4 rotation;
    if (b->num_rotations == 1)
    {
        vec4 norm;
        glm_normalize_to(b->rotations[0].rotation, norm);
        glm_quat_mat4(norm, rotation);
    }
    else
        interpolate_rotation(animation_time, b->rotations[rot_index], b->rotations[rot_index + 1], rotation);

    size_t scl_index = get_scale_index(b, animation_time);
    mat4 scale;
    if (b->num_scalings == 1)
        glm_scale_to(GLM_MAT4_IDENTITY, b->scalings[0].scale, scale);
    else
        interpolate_scaling(animation_time, b->scalings[scl_index], b->scalings[scl_index + 1], scale);

    glm_mat4_mulN((mat4 *[]){&translation, &rotation, &scale}, 3, b->transform);
}

#endif /* BONE_H */