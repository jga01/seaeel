#ifndef ENTITY_H
#define ENTITY_H

#include "cglm/cglm.h"

enum EntityType {
    PLAYER,
    OBJECT,
};

struct Entity {
    vec3 position, velocity;
};

#endif /* ENTITY_H */