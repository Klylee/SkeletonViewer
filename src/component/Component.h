#pragma once
#include <glm/glm.hpp>

struct Component
{
    virtual ~Component() = default;
};

struct Transform : public Component
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct MeshRenderer : public Component
{
    unsigned int meshID;
    unsigned int materialID;
};

struct LightComponent : public Component
{
    enum class LightType
    {
        Directional,
        Point,
        Spot
    };

    LightType type;
    glm::vec3 color;
    float intensity;
};
