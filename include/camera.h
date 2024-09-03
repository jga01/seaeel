#ifndef CAMERA_H
#define CAMERA_H

#include "cglm/cglm.h"

enum camera_movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

typedef struct
{
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 world_up;
    float fov;
    float near_clip;
    float far_clip;
    float yaw;
    float pitch;
    float movement_speed;
    float mouse_sensitivity;
    float zoom;
} camera;

void print_vec3(vec3 v)
{
    printf("vec3: [%.2f, %.2f, %.2f]\n", v[0], v[1], v[2]);
}

void camera_debug(camera *camera, const char *context)
{
    printf("%s\n\n", context);
    printf("Camera Position:\n");
    print_vec3(camera->position);

    printf("Camera Front:\n");
    print_vec3(camera->front);

    printf("Camera Right:\n");
    print_vec3(camera->right);

    printf("Camera Up:\n");
    print_vec3(camera->up);

    printf("Yaw: %.2f, Pitch: %.2f\n", camera->yaw, camera->pitch);
    printf("Zoom: %.2f\n", camera->zoom);
    printf("\n");
}

static void camera_update_vectors(camera *c)
{
    vec3 front;
    front[0] = cos(glm_rad(c->yaw)) * cos(glm_rad(c->pitch));
    front[1] = sin(glm_rad(c->pitch));
    front[2] = sin(glm_rad(c->yaw)) * cos(glm_rad(c->pitch));
    glm_normalize_to(front, c->front);
    glm_vec3_cross(c->front, c->world_up, c->right);
    glm_vec3_normalize(c->right);
    glm_vec3_cross(c->right, c->front, c->up);
    glm_vec3_normalize(c->up);
}

void camera_init(camera *c)
{
    glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, c->position);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, c->front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, c->world_up);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, c->up);
    c->fov = 45.0f;
    c->near_clip = 0.1f;
    c->far_clip = 100.0f;
    c->yaw = YAW;
    c->pitch = PITCH;
    c->movement_speed = SPEED;
    c->mouse_sensitivity = SENSITIVITY;
    c->zoom = ZOOM;
    camera_update_vectors(c);
}

void camera_get_view_matrix(camera *c, mat4 m)
{
    vec3 center;
    glm_vec3_add(c->position, c->front, center);
    glm_lookat(c->position, center, c->up, m);
}

void camera_process_keyboard(camera *c, enum camera_movement direction, float delta)
{
    float velocity = c->movement_speed * delta;
    if (direction == FORWARD)
        glm_vec3_muladds(c->front, velocity, c->position);
    if (direction == BACKWARD)
        glm_vec3_mulsubs(c->front, velocity, c->position);
    if (direction == LEFT)
        glm_vec3_mulsubs(c->right, velocity, c->position);
    if (direction == RIGHT)
        glm_vec3_muladds(c->right, velocity, c->position);
}

void camera_process_mouse_movement(camera *c, float xoffset, float yoffset, unsigned char constrain_pitch)
{
    xoffset *= c->mouse_sensitivity;
    yoffset *= c->mouse_sensitivity;

    c->yaw += xoffset;
    c->pitch += yoffset;

    if (constrain_pitch)
    {
        if (c->pitch > 89.0f)
            c->pitch = 89.0f;
        if (c->pitch < -89.0f)
            c->pitch = -89.0f;
    }

    camera_update_vectors(c);
}

void camera_process_mouse_scroll(camera *c, float yoffset)
{
    c->zoom -= (float)yoffset;
    if (c->zoom < 1.0f)
        c->zoom = 1.0f;
    if (c->zoom > 45.0f)
        c->zoom = 45.0f;
}

#endif /* CAMERA_H */