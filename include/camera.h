#ifndef CAMERA_H
#define CAMERA_H

#include "cglm/cglm.h"

enum CameraMovement
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

struct Camera
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
};

static void seel_camera_update_vectors(struct Camera *camera)
{
    vec3 front;
    front[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    front[1] = sin(glm_rad(camera->pitch));
    front[2] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    glm_normalize_to(front, camera->front);
    glm_vec3_cross(camera->front, camera->world_up, camera->right);
    glm_vec3_normalize(camera->right);
    glm_vec3_cross(camera->right, camera->front, camera->up);
    glm_vec3_normalize(camera->up);
}

struct Camera seel_camera_create(void)
{
    struct Camera camera;

    glm_vec3_copy((vec3){0.0f, 0.0f, 3.0f}, camera.position);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, camera.front);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera.world_up);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, camera.up);
    camera.fov = 45.0f;
    camera.near_clip = 0.1f;
    camera.far_clip = 100.0f;
    camera.yaw = YAW;
    camera.pitch = PITCH;
    camera.movement_speed = SPEED;
    camera.mouse_sensitivity = SENSITIVITY;
    camera.zoom = ZOOM;
    seel_camera_update_vectors(&camera);

    return camera;
}

void seel_camera_get_view_matrix(struct Camera *camera, mat4 matrix)
{
    vec3 center;
    glm_vec3_add(camera->position, camera->front, center);
    glm_lookat(camera->position, center, camera->up, matrix);
}

void seel_camera_process_keyboard(struct Camera *c, enum CameraMovement direction, float delta)
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

void seel_camera_process_mouse_movement(struct Camera *camera, float xoffset, float yoffset, unsigned char constrain_pitch)
{
    xoffset *= camera->mouse_sensitivity;
    yoffset *= camera->mouse_sensitivity;

    camera->yaw += xoffset;
    camera->pitch += yoffset;

    if (constrain_pitch)
    {
        if (camera->pitch > 89.0f)
            camera->pitch = 89.0f;
        if (camera->pitch < -89.0f)
            camera->pitch = -89.0f;
    }

    seel_camera_update_vectors(camera);
}

void seel_camera_process_mouse_scroll(struct Camera *camera, float yoffset)
{
    camera->zoom -= (float)yoffset;
    if (camera->zoom < 1.0f)
        camera->zoom = 1.0f;
    if (camera->zoom > 45.0f)
        camera->zoom = 45.0f;
}

#endif /* CAMERA_H */