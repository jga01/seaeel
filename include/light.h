#ifndef LIGHT_H
#define LIGHT_H

#include "cglm/cglm.h"

typedef struct
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} directional_light;

typedef struct
{
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} point_light;

typedef struct
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} spot_light;

#endif /* LIGHT_H */