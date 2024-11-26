#ifndef PARTICLE_H
#define PARTICLE_H

#include "cglm/cglm.h"

#define MAX_PARTICLES 500

struct Particle
{
    vec2 position, velocity;
    vec4 color;
    float life;
};

struct ParticleManager
{
    struct Particle particles[MAX_PARTICLES];
    unsigned int last_used_particle;
    unsigned int particles_alive;
};

struct Particle seel_particle_create(void)
{
    return (struct Particle){
        .color = {1.0f, 1.0f, 1.0f, 1.0f},
        .position = {0.0f, 0.0f},
        .velocity = {0.0f, 0.0f},
        .life = 0.0f};
};

void seel_particles_init(struct ParticleManager *pmanager)
{
    unsigned int i;
    for (i = 0; i < MAX_PARTICLES; i++)
    {
        pmanager->particles[i] = seel_particle_create();
    }
};

unsigned int first_unused_particle(struct ParticleManager *pmanager)
{
    unsigned int i;
    for (i = pmanager->last_used_particle; i < MAX_PARTICLES; i++)
    {
        if (pmanager->particles[i].life <= 0.0f)
        {
            pmanager->last_used_particle = i;
            return i;
        }
    }

    for (i = 0; i < pmanager->last_used_particle; i++)
    {
        if (pmanager->particles[i].life <= 0.0f)
        {
            pmanager->last_used_particle = i;
            return i;
        }
    }

    pmanager->last_used_particle = 0;
    return 0;
}

void respawn_particle(struct Particle *particle, vec2 offset)
{
    float random = ((rand() % 100) - 50) / 10.0f;
    float color = 0.5f + ((rand() % 100) / 100.0f);
}

void seel_particles_process(struct ParticleManager *pmanager, float dt)
{
    unsigned int new_particles = 2;
    unsigned int i;
    for (i = 0; i < new_particles; i++)
    {
        int unused_particle = first_unused_particle(pmanager);
        // respawn_particle(&pmanager->particles[unused_particle], object, offset);
    }

    for (i = 0; i < MAX_PARTICLES; i++)
    {
        struct Particle *p = &pmanager->particles[i];
        p->life -= dt;
        if (p->life > 0.0f)
        {
            vec2 temp;
            glm_vec2_scale(p->velocity, dt, temp);
            glm_vec2_sub(p->position, temp, p->position);
            p->color[3] -= dt * 2.5f;
        }
    }
}

#endif /* PARTICLE_H */