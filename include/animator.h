#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "cglm/cglm.h"
#include "animation.h"
#include "bone.h"

typedef struct animator
{
    mat4 *final_bone_matrices;
    int num_matrices;
    animation *current_animation;
    animation *next_animation;
    animation *queue_animation;
    float current_time;
    int interpolating;
    float halt_time;
    float inter_time;
} animator;

struct key_position get_positions(bone *b, float animation_time)
{
    size_t pos_index = (animation_time == 0.0f) ? 0 : get_position_index(b, animation_time) + 1;
    return b->positions[pos_index];
}

struct key_rotation get_rotations(bone *b, float animation_time)
{
    size_t rot_index = (animation_time == 0.0f) ? 0 : get_rotation_index(b, animation_time) + 1;
    return b->rotations[rot_index];
}

struct key_scaling get_scalings(bone *b, float animation_time)
{
    size_t scl_index = (animation_time == 0.0f) ? 0 : get_scale_index(b, animation_time) + 1;
    return b->scalings[scl_index];
}

void calculate_bone_transition(animator *a, struct assimp_node_data *cur_node, vec4 *parent_transform, animation *prev_animation, animation *next_animation, float halt_time, float current_time, float transition_time)
{
    char *node_name = cur_node->name;
    mat4 transform;
    glm_mat4_copy(cur_node->transformation, transform);

    bone prev_bone = find_bone(prev_animation, node_name);
    bone next_bone = find_bone(next_animation, node_name);

    if (prev_bone.id != -1 && next_bone.id != -1)
    {
        struct key_position prev_pos = get_positions(&prev_bone, halt_time);
        struct key_rotation prev_rot = get_rotations(&prev_bone, halt_time);
        struct key_scaling prev_scl = get_scalings(&prev_bone, halt_time);

        struct key_position next_pos = get_positions(&next_bone, 0.0f);
        struct key_rotation next_rot = get_rotations(&next_bone, 0.0f);
        struct key_scaling next_scl = get_scalings(&next_bone, 0.0f);

        prev_pos.time_stamp = 0.0f;
        prev_rot.time_stamp = 0.0f;
        prev_scl.time_stamp = 0.0f;

        next_pos.time_stamp = transition_time;
        next_rot.time_stamp = transition_time;
        next_scl.time_stamp = transition_time;

        mat4 p, r, s;
        interpolate_position(current_time, prev_pos, next_pos, p);
        interpolate_rotation(current_time, prev_rot, next_rot, r);
        interpolate_scaling(current_time, prev_scl, next_scl, s);

        glm_mat4_mulN((mat4 *[]){&p, &r, &s}, 3, transform);
    }

    mat4 global_transformation;
    glm_mat4_mul(parent_transform, transform, global_transformation);

    bone_info *bone_inf = next_animation->bone_info;
    int bone_info_size = next_animation->num_bones;
    for (unsigned int i = 0; i < bone_info_size; i++)
    {
        if (!strcmp(bone_inf[i].name, node_name))
        {
            mat4 offset;
            glm_mat4_copy(bone_inf[i].offset, offset);
            glm_mat4_mul(global_transformation, offset, a->final_bone_matrices[i]);
            break;
        }
    }

    for (int i = 0; i < cur_node->children_count; i++)
        calculate_bone_transition(a, &cur_node->children[i], global_transformation, prev_animation, next_animation, halt_time, current_time, transition_time);
}

void calculate_bone_transform(animator *a, struct assimp_node_data *node, vec4 *parent_transform, animation *animation, float current_time)
{
    char *node_name = node->name;
    mat4 bone_transform;
    glm_mat4_copy(node->transformation, bone_transform);

    bone b = find_bone(animation, node_name);
    // printf("bone name: %s\n", b.name);

    if (b.id != -1)
    {
        update(&b, current_time);
        glm_mat4_copy(b.transform, bone_transform);
        // glm_mat4_print(bone_transform, stdout);
    }

    mat4 global_transformation;
    glm_mat4_mul(parent_transform, bone_transform, global_transformation);

    bone_info *bone_inf = animation->bone_info;
    int bone_info_size = animation->num_bones;

    for (unsigned int i = 0; i < bone_info_size; i++)
    {
        if (!strcmp(bone_inf[i].name, node_name))
        {
            mat4 offset;
            glm_mat4_copy(bone_inf[i].offset, offset);
            glm_mat4_mul(global_transformation, offset, a->final_bone_matrices[i]);
            break;
        }
    }

    for (int i = 0; i < node->children_count; i++)
        calculate_bone_transform(a, &node->children[i], global_transformation, animation, current_time);
}

void update_animation(animator *a, float dt)
{
    if (a->current_animation)
    {
        a->current_time = fmod(a->current_time + a->current_animation->ticks_per_second * dt, a->current_animation->duration);

        float transition_time = a->current_animation->ticks_per_second * 0.2f;
        if (a->interpolating && a->inter_time <= transition_time)
        {
            a->inter_time += a->current_animation->ticks_per_second * dt;
            calculate_bone_transition(a, &a->current_animation->root_node, GLM_MAT4_IDENTITY, a->current_animation, a->next_animation, a->halt_time, a->inter_time, transition_time);
            return;
        }
        else if (a->interpolating)
        {
            if (a->queue_animation)
            {
                a->current_animation = a->next_animation;
                a->halt_time = 0.0f;
                a->next_animation = a->queue_animation;
                a->queue_animation = NULL;
                a->current_time = 0.0f;
                a->inter_time = 0.0;
                return;
            }

            a->interpolating = false;
            a->current_animation = a->next_animation;
            a->current_time = 0.0;
            a->inter_time = 0.0;
        }

        calculate_bone_transform(a, &a->current_animation->root_node, GLM_MAT4_IDENTITY, a->current_animation, a->current_time);
    }
}

void play_animation(animator *a, animation *pAnimation, int repeat)
{
    if (!a->current_animation)
    {
        a->current_animation = pAnimation;
        return;
    }

    if (a->interpolating)
    {
        // Handle interpolating from current interpolation here
        if (pAnimation != a->next_animation)
            a->queue_animation = pAnimation;
    }
    else
    {
        // Else: Just playing current animation
        // Start interpolation
        if (pAnimation != a->next_animation)
        {
            a->interpolating = true;
            a->halt_time = fmod(a->current_time, a->current_animation->duration);
            a->next_animation = pAnimation;
            a->current_time = 0.0f;
            a->inter_time = 0.0;
        }
    }
}

void animator_create(animator *anim)
{
    anim->final_bone_matrices = calloc(100, sizeof(mat4));
    if (!anim->final_bone_matrices)
    {
        fprintf(stderr, "Failed to allocate memory for final bone matrices!\n");
        return;
    }

    for (unsigned int i = 0; i < 100; i++)
    {
        glm_mat4_copy(GLM_MAT4_IDENTITY, anim->final_bone_matrices[i]);
    }

    anim->num_matrices = 100;
}

void print_matrix(const float matrix[4][4], const char *title)
{
    printf("%s:\n", title);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("%6.2f ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void print_debug_info(const animator *a)
{
    if (!a)
    {
        printf("Animator is NULL\n");
        return;
    }

    printf("=== Animator Debug Info ===\n");

    // Print Animator State
    printf("Animator State:\n");
    printf("Number of Final Bone Matrices: %d\n", a->num_matrices);
    printf("Current Time: %f\n", a->current_time);
    printf("Interpolating: %s\n", a->interpolating ? "Yes" : "No");
    printf("Halt Time: %f\n", a->halt_time);
    printf("Interpolation Time: %f\n", a->inter_time);

    // Print Final Bone Matrices
    printf("\nFinal Bone Matrices:\n");
    for (int i = 0; i < a->num_matrices; i++)
    {
        printf("Matrix %d:\n", i);
        print_matrix(a->final_bone_matrices[i], "");
    }

    // Print Current Animation
    if (a->current_animation)
    {
        printf("\nCurrent Animation:\n");
        printf("Duration: %f\n", a->current_animation->duration);
        printf("Ticks Per Second: %d\n", a->current_animation->ticks_per_second);
        printf("Number of Bones: %d\n", a->current_animation->num_bones);

        for (int i = 0; i < a->current_animation->num_bones; i++)
        {
            const bone *b = &a->current_animation->bones[i];
            printf("\nBone %d: %s\n", i, b->name);
            printf("ID: %d\n", b->id);
            printf("Number of Positions: %d\n", b->num_positions);
            printf("Number of Rotations: %d\n", b->num_rotations);
            printf("Number of Scalings: %d\n", b->num_scalings);

            printf("Positions:\n");
            for (int j = 0; j < 1; j++)
            {
                printf("  Position %d: [%f, %f, %f] at time %f\n",
                       j, b->positions[j].position[0], b->positions[j].position[1], b->positions[j].position[2], b->positions[j].time_stamp);
            }

            printf("Rotations:\n");
            for (int j = 0; j < 1; j++)
            {
                printf("  Rotation %d: [%f, %f, %f, %f] at time %f\n",
                       j, b->rotations[j].rotation[0], b->rotations[j].rotation[1], b->rotations[j].rotation[2], b->rotations[j].rotation[3], b->rotations[j].time_stamp);
            }

            printf("Scalings:\n");
            for (int j = 0; j < 1; j++)
            {
                printf("  Scaling %d: [%f, %f, %f] at time %f\n",
                       j, b->scalings[j].scale[0], b->scalings[j].scale[1], b->scalings[j].scale[2], b->scalings[j].time_stamp);
            }

            print_matrix(b->transform, "Transform Matrix");
        }
    }

    // Print Next Animation
    if (a->next_animation)
    {
        printf("\nNext Animation:\n");
        printf("Duration: %f\n", a->next_animation->duration);
        printf("Ticks Per Second: %d\n", a->next_animation->ticks_per_second);
        printf("Number of Bones: %d\n", a->next_animation->num_bones);
        // Similar details as current_animation can be printed here
    }

    // Print Queue Animation
    if (a->queue_animation)
    {
        printf("\nQueued Animation:\n");
        printf("Duration: %f\n", a->queue_animation->duration);
        printf("Ticks Per Second: %d\n", a->queue_animation->ticks_per_second);
        printf("Number of Bones: %d\n", a->queue_animation->num_bones);
        // Similar details as current_animation can be printed here
    }

    printf("===========================\n");
}

#endif /* ANIMATOR_H */