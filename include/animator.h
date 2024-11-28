#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "cglm/cglm.h"
#include "animation.h"
#include "bone.h"

struct Animator
{
    mat4 *final_bone_matrices;
    int num_matrices;
    struct Animation *current_animation;
    struct Animation *next_animation;
    struct Animation *queue_animation;
    float current_time;
    float halt_time;
    float inter_time;
    unsigned int interpolating;
};

struct KeyPosition seel_get_positions(struct Bone *bone, float animation_time)
{
    unsigned int pos_index = (animation_time == 0.0f) ? 0 : seel_get_position_index(bone, animation_time) + 1;
    return bone->positions[pos_index];
}

struct KeyRotation seel_get_rotations(struct Bone *bone, float animation_time)
{
    unsigned int rot_index = (animation_time == 0.0f) ? 0 : seel_get_rotation_index(bone, animation_time) + 1;
    return bone->rotations[rot_index];
}

struct KeyScale seel_get_scalings(struct Bone *bone, float animation_time)
{
    unsigned int scl_index = (animation_time == 0.0f) ? 0 : seel_get_scale_index(bone, animation_time) + 1;
    return bone->scalings[scl_index];
}

void seel_calculate_bone_transition(struct Animator *animator,
                                    struct AssimpNodeData *cur_node,
                                    vec4 *parent_transform,
                                    struct Animation *prev_animation,
                                    struct Animation *next_animation,
                                    float halt_time,
                                    float current_time,
                                    float transition_time)
{
    char *node_name = cur_node->name;
    mat4 transform;
    glm_mat4_copy(cur_node->transformation, transform);

    struct Bone prev_bone = seel_find_bone(prev_animation, node_name);
    struct Bone next_bone = seel_find_bone(next_animation, node_name);

    if (prev_bone.id != -1 && next_bone.id != -1)
    {
        struct KeyPosition prev_pos = seel_get_positions(&prev_bone, halt_time);
        struct KeyRotation prev_rot = seel_get_rotations(&prev_bone, halt_time);
        struct KeyScale prev_scl = seel_get_scalings(&prev_bone, halt_time);

        struct KeyPosition next_pos = seel_get_positions(&next_bone, 0.0f);
        struct KeyRotation next_rot = seel_get_rotations(&next_bone, 0.0f);
        struct KeyScale next_scl = seel_get_scalings(&next_bone, 0.0f);

        prev_pos.time_stamp = 0.0f;
        prev_rot.time_stamp = 0.0f;
        prev_scl.time_stamp = 0.0f;

        next_pos.time_stamp = transition_time;
        next_rot.time_stamp = transition_time;
        next_scl.time_stamp = transition_time;

        mat4 position, rotation, scale;
        seel_interpolate_position(current_time, prev_pos, next_pos, position);
        seel_interpolate_rotation(current_time, prev_rot, next_rot, rotation);
        seel_interpolate_scaling(current_time, prev_scl, next_scl, scale);

        glm_mat4_mulN((mat4 *[]){&position, &rotation, &scale}, 3, transform);
    }

    mat4 global_transformation;
    glm_mat4_mul(parent_transform, transform, global_transformation);

    struct BoneInfo *bone_info = next_animation->bone_info;
    unsigned int bone_info_size = next_animation->num_bones;

    unsigned int i;
    for (i = 0; i < bone_info_size; i++)
    {
        if (!strcmp(bone_info[i].name, node_name))
        {
            mat4 offset;
            glm_mat4_copy(bone_info[i].offset, offset);
            glm_mat4_mul(global_transformation, offset, animator->final_bone_matrices[i]);
            break;
        }
    }

    for (i = 0; i < cur_node->children_count; i++)
        seel_calculate_bone_transition(animator,
                                       &cur_node->children[i],
                                       global_transformation,
                                       prev_animation,
                                       next_animation,
                                       halt_time,
                                       current_time,
                                       transition_time);
}

/**
 * Updates the bone transformations for the animator based on the current time.
 * Recursively traverses the hierarchy of Assimp nodes and updates the matrices
 * of bones present in the animation.
 * 
 * @param animator - Pointer to the Animator instance.
 * @param node - Current AssimpNodeData being processed.
 * @param parent_transform - Transform of the parent node.
 * @param animation - Pointer to the active Animation.
 * @param current_time - Current time in the animation cycle.
 */
void seel_calculate_bone_transform(struct Animator *animator,
                                   struct AssimpNodeData *node,
                                   vec4 *parent_transform,
                                   struct Animation *animation,
                                   float current_time)
{
    char *node_name = node->name;
    mat4 bone_transform;
    glm_mat4_copy(node->transformation, bone_transform);

    printf("%s\n", node_name);

    struct Bone bone = seel_find_bone(animation, node_name);

    if (bone.id != -1)
    {
        seel_bone_update(&bone, current_time);
        glm_mat4_copy(bone.transform, bone_transform);
    }

    mat4 global_transformation;
    glm_mat4_mul(parent_transform, bone_transform, global_transformation);

    struct BoneInfo *bone_info = animation->bone_info;
    unsigned int bone_info_size = animation->num_bones;

    unsigned int i;
    for (i = 0; i < bone_info_size; i++)
    {
        if (!strcmp(bone_info[i].name, node_name))
        {
            mat4 offset;
            glm_mat4_copy(bone_info[i].offset, offset);
            glm_mat4_mul(global_transformation, offset, animator->final_bone_matrices[i]);
            break;
        }
    }

    for (i = 0; i < node->children_count; i++)
        seel_calculate_bone_transform(animator, &node->children[i], global_transformation, animation, current_time);
}

void seel_update_animation(struct Animator *animator, float delta_time)
{
    if (animator->current_animation)
    {
        animator->current_time = fmod(animator->current_time + animator->current_animation->ticks_per_second * delta_time, animator->current_animation->duration);

        float transition_time = animator->current_animation->ticks_per_second * 0.2f;
        if (animator->interpolating && animator->inter_time <= transition_time)
        {
            animator->inter_time += animator->current_animation->ticks_per_second * delta_time;
            seel_calculate_bone_transition(animator,
                                           &animator->current_animation->root_node,
                                           GLM_MAT4_IDENTITY,
                                           animator->current_animation,
                                           animator->next_animation,
                                           animator->halt_time,
                                           animator->inter_time,
                                           transition_time);
            return;
        }
        else if (animator->interpolating)
        {
            if (animator->queue_animation)
            {
                animator->current_animation = animator->next_animation;
                animator->halt_time = 0.0f;
                animator->next_animation = animator->queue_animation;
                animator->queue_animation = NULL;
                animator->current_time = 0.0f;
                animator->inter_time = 0.0;
                return;
            }

            animator->interpolating = false;
            animator->current_animation = animator->next_animation;
            animator->current_time = 0.0;
            animator->inter_time = 0.0;
        }

        seel_calculate_bone_transform(animator, &animator->current_animation->root_node, GLM_MAT4_IDENTITY, animator->current_animation, animator->current_time);
    }
}

void seel_play_animation(struct Animator *animator, struct Animation *pAnimation, unsigned int repeat)
{
    if (!animator->current_animation)
    {
        animator->current_animation = pAnimation;
        return;
    }

    if (animator->interpolating)
    {
        /* Handle interpolating from current interpolation here */
        if (pAnimation != animator->next_animation)
            animator->queue_animation = pAnimation;
    }
    else
    {
        /*
         * Else: Just playing current animation
         * Start interpolation
         */
        if (pAnimation != animator->next_animation)
        {
            animator->interpolating = true;
            animator->halt_time = fmod(animator->current_time, animator->current_animation->duration);
            animator->next_animation = pAnimation;
            animator->current_time = 0.0f;
            animator->inter_time = 0.0;
        }
    }
}

void seel_animator_create(struct Animator *animator)
{
    memset(animator, 0, sizeof(struct Animator));

    animator->final_bone_matrices = calloc(MAX_BONES, sizeof(mat4));
    if (!animator->final_bone_matrices)
    {
        fprintf(stderr, "Failed to allocate memory for final bone matrices!\n");
        return;
    }

    unsigned int i;
    for (i = 0; i < MAX_BONES; i++)
    {
        glm_mat4_copy(GLM_MAT4_IDENTITY, animator->final_bone_matrices[i]);
    }

    animator->num_matrices = MAX_BONES;
}

#endif /* ANIMATOR_H */