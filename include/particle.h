#ifndef PARTICLE_H
#define PARTICLE_H

#include "cglm/cglm.h"
#include "GLFW/glfw3.h"
#include "glad/gl.h"
#include "texture.h"
#include "camera.h"

// Particle structure to hold individual particle data
struct Particle
{
    vec3 position;
    vec3 velocity;
    vec3 color;
    float life;
    float size;
    float initial_life; // To track percentage of life remaining
};

// Particle system structure
struct ParticleEmitter
{
    struct Particle *particles;
    struct Shader *shader;
    struct Texture *texture;
    unsigned int max_particles;
    unsigned int active_particles;
    unsigned int last_used_particle;

    // Emission properties
    float spawn_rate; // Particles per second
    vec3 position;    // Emitter position
    vec3 gravity;

    // GPU resources
    unsigned int VAO;
    unsigned int vertices_VBO;
    unsigned int instance_VBO;
};

// Initialize the particle system
struct ParticleEmitter seel_particle_emitter_create(struct Shader *shader, struct Texture *texture,
                                                     unsigned int max_particles, vec3 position)
{
    struct ParticleEmitter emitter;
    emitter.shader = shader;
    emitter.texture = texture;
    emitter.max_particles = max_particles;
    emitter.active_particles = 0;
    emitter.last_used_particle = 0;
    emitter.spawn_rate = 50.0f; // Default spawn rate
    glm_vec3_copy(position, emitter.position);
    glm_vec3_copy((vec3){0.0f, -9.81f, 0.0f}, emitter.gravity);

    // Allocate particle array
    emitter.particles = malloc(sizeof(struct Particle) * max_particles);

    // Initialize OpenGL buffers
    float quad_vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f};

    glGenVertexArrays(1, &emitter.VAO);
    glGenBuffers(1, &emitter.vertices_VBO);
    glGenBuffers(1, &emitter.instance_VBO);

    glBindVertexArray(emitter.VAO);

    // Set up vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, emitter.vertices_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    // Position and texture coordinates
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    // Instance data buffer (will be updated each frame)
    glBindBuffer(GL_ARRAY_BUFFER, emitter.instance_VBO);
    glBufferData(GL_ARRAY_BUFFER, max_particles * (6 * sizeof(float) + 4 * sizeof(float)), NULL, GL_STREAM_DRAW);

    // Instance position and size (vec4)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)0);
    glVertexAttribDivisor(2, 1);

    // Instance color (vec3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(4 * sizeof(float)));
    glVertexAttribDivisor(3, 1);

    // Instance life (float)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void *)(7 * sizeof(float)));
    glVertexAttribDivisor(4, 1);

    glBindVertexArray(0);

    return emitter;
}

// Find the next available particle slot
static int find_unused_particle(struct ParticleEmitter *emitter)
{
    // First search from last used particle
    for (unsigned int i = emitter->last_used_particle; i < emitter->max_particles; i++)
    {
        if (emitter->particles[i].life <= 0.0f)
        {
            emitter->last_used_particle = i;
            return i;
        }
    }

    // Search from beginning
    for (unsigned int i = 0; i < emitter->last_used_particle; i++)
    {
        if (emitter->particles[i].life <= 0.0f)
        {
            emitter->last_used_particle = i;
            return i;
        }
    }

    // If no dead particle found, override the first one
    emitter->last_used_particle = 0;
    return 0;
}

// Spawn a new particle
static void spawn_particle(struct ParticleEmitter *emitter)
{
    int index = find_unused_particle(emitter);
    struct Particle *p = &emitter->particles[index];

    // Position slightly randomized around emitter
    p->position[0] = emitter->position[0] + ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 0.1f;
    p->position[1] = emitter->position[1] + ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 0.1f;
    p->position[2] = emitter->position[2] + ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 0.1f;

    // Random velocity
    float spread = 0.5f;
    p->velocity[0] = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;
    p->velocity[1] = ((float)rand() / RAND_MAX) * -10.0f; // Upward bias
    p->velocity[2] = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * spread;

    // Random color (warm colors for fire-like effect)
    p->color[0] = 0.5f + ((float)rand() / RAND_MAX) * 0.5f; // Red
    p->color[1] = 0.2f + ((float)rand() / RAND_MAX) * 0.3f; // Green
    p->color[2] = 0.0f + ((float)rand() / RAND_MAX) * 0.2f; // Blue

    p->life = 1.0f + ((float)rand() / RAND_MAX) * 0.5f; // 1-1.5 seconds
    p->initial_life = p->life;
    p->size = 0.1f + ((float)rand() / RAND_MAX) * 0.2f;

    if (emitter->active_particles < emitter->max_particles)
    {
        emitter->active_particles++;
    }
}

// Update all particles and prepare instance data
void seel_particle_emitter_update(struct ParticleEmitter *emitter, float delta_time)
{
    // Spawn new particles based on spawn rate
    float particles_to_spawn = emitter->spawn_rate * delta_time;
    int whole_particles = (int)particles_to_spawn;
    float fractional_part = particles_to_spawn - whole_particles;

    // Spawn whole particles
    for (int i = 0; i < whole_particles; i++)
    {
        spawn_particle(emitter);
    }

    // Probabilistically spawn one more based on fractional part
    if ((float)rand() / RAND_MAX < fractional_part)
    {
        spawn_particle(emitter);
    }

    // Update existing particles
    float *instance_data = malloc(emitter->active_particles * (6 * sizeof(float) + 4 * sizeof(float)));
    int alive_count = 0;

    for (unsigned int i = 0; i < emitter->max_particles; i++)
    {
        struct Particle *p = &emitter->particles[i];

        if (p->life > 0.0f)
        {
            // Update physics
            p->life -= delta_time;
            glm_vec3_scale(emitter->gravity, delta_time, p->velocity);
            glm_vec3_add(p->position, p->velocity, p->position);

            if (p->life > 0.0f)
            {
                // Fill instance data (position, size, color, life)
                int offset = alive_count * 10;
                memcpy(&instance_data[offset], p->position, 3 * sizeof(float));
                instance_data[offset + 3] = p->size;
                memcpy(&instance_data[offset + 4], p->color, 3 * sizeof(float));
                instance_data[offset + 7] = p->life / p->initial_life;
                alive_count++;
            }
            else
            {
                emitter->active_particles--;
            }
        }
    }

    // Update instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, emitter->instance_VBO);
    glBufferData(GL_ARRAY_BUFFER, alive_count * 10 * sizeof(float), instance_data, GL_STREAM_DRAW);
    free(instance_data);
}

// Render all active particles
void seel_particle_emitter_render(struct ParticleEmitter *emitter, struct Camera *camera)
{
    if (emitter->active_particles == 0)
        return;

    // Calculate view-projection matrix
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    mat4 projection = GLM_MAT4_IDENTITY_INIT;
    mat4 vp = GLM_MAT4_IDENTITY_INIT;

    seel_camera_get_view_matrix(camera, view);
    glm_perspective(glm_rad(camera->fov), camera->aspect_ratio,
                    camera->near_clip, camera->far_clip, projection);
    glm_mat4_mul(projection, view, vp);

    // Set up rendering state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glow effect

    seel_shader_use(emitter->shader);
    seel_shader_set_mat4(emitter->shader, "vp", &vp[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, emitter->texture->id);
    seel_shader_set_int(emitter->shader, "sprite", 0);

    // Draw all particles using instancing
    glBindVertexArray(emitter->VAO);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, emitter->active_particles);
    glBindVertexArray(0);

    // Reset state
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glDisable(GL_BLEND);
}

// Clean up resources
void seel_particle_emitter_destroy(struct ParticleEmitter *emitter)
{
    glDeleteVertexArrays(1, &emitter->VAO);
    glDeleteBuffers(1, &emitter->vertices_VBO);
    glDeleteBuffers(1, &emitter->instance_VBO);
    free(emitter->particles);
    free(emitter);
}

#endif /* PARTICLE_H */