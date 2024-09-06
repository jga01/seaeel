#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#define MAX_KEYS 1024

enum GameState
{
    GAME_ACTIVE,
    GAME_MENU
};

struct Engine
{
    bool keys[MAX_KEYS];
    unsigned int width, height;
};

struct Engine seel_engine_create(unsigned int width, unsigned int height);
void seel_engine_init(struct Engine *e);
void seel_engine_update(float delta_time);

#endif /* ENGINE_H */