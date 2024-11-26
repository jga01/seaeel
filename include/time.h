#ifndef TIME_H
#define TIME_H

#include "GLFW/glfw3.h"

struct TimeManager
{
    float delta_time;
    float last_time;
    float current_time;
    int frame_count;
    float last_frame_time;
    float frame_rate;
};

void seel_time_init(struct TimeManager *manager);
void seel_time_update(struct TimeManager *manager);

void seel_time_init(struct TimeManager *manager)
{
    manager->delta_time = 0.0f;
    manager->last_time = (float)glfwGetTime();
    manager->current_time = manager->last_time;
    manager->frame_count = 0;
    manager->last_frame_time = manager->last_time;
    manager->frame_rate = 0.0f;
}

void seel_time_update(struct TimeManager *manager)
{
    manager->current_time = (float)glfwGetTime();
    manager->delta_time = manager->current_time - manager->last_time;
    manager->last_time = manager->current_time;

    manager->frame_count++;
    if (manager->current_time - manager->last_frame_time >= 1.0f)
    {
        manager->frame_rate = (float)manager->frame_count / (manager->current_time - manager->last_frame_time);
        manager->last_frame_time = manager->current_time;
        manager->frame_count = 0;
    }
}

#endif /* TIME_H*/