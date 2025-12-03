#pragma once
#include "SceneObject.h"

enum class LightType
{
    Directional,
    Point,
    Spot
};

class Light : public SceneObject
{
public:
    REGISTER_SCENE_OBJECT(Light)

    LightType type;
    float intensity = 1.0f;
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
};

class DirectionalLight : public Light
{
public:
    REGISTER_SCENE_OBJECT(DirectionalLight)
    DirectionalLight() { type = LightType::Directional; }
    glm::vec3 direction = {0, -1, 0};
};

class PointLight : public Light
{
public:
    REGISTER_SCENE_OBJECT(PointLight)
    PointLight() { type = LightType::Point; }
    float range = 10.0f;
};

class SpotLight : public Light
{
public:
    REGISTER_SCENE_OBJECT(SpotLight)
    SpotLight() { type = LightType::Spot; }
    glm::vec3 direction = {0, -1, 0};
    float innerCone = glm::radians(15.0f);
    float outerCone = glm::radians(30.0f);
    float range = 15.0f;
};
