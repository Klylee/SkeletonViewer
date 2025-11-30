#pragma once
#include "SceneObject.h"

class Light : public SceneObject
{
public:
    enum class Type
    {
        Directional,
        Point,
        Spot
    };

    REGISTER_SCENE_OBJECT(Light)

    Type type;
    float intensity = 1.0f;
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    float range = 10.0f;     // 仅对点光源和聚光灯有效
    float spotAngle = 30.0f; // 仅对聚光灯
};