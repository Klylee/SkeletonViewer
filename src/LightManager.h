#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <GL/glew.h>
#include "Light.h"

constexpr int MAX_LIGHTS = 32;

struct GPULight
{
    int type; // 0=Directional,1=Point,2=Spot
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;

    float range;     // for point/spot
    float innerCone; // for spot
    float outerCone; // for spot
    float padding0;
    float padding1;
};

struct LightUBO
{
    // 4 + 3 * 4
    int lightCount;
    int padding[3]; // maintain 16 bytes alignment

    // Light_num * 64
    GPULight lights[MAX_LIGHTS];
};

class LightManager
{
public:
    GLuint ubo = 0;
    static constexpr int MAX_LIGHTS = 32;

    std::vector<std::weak_ptr<Light>> lights; // actual lights in scene
    LightUBO uboData;                         // CPU-side buffer

    LightManager()
    {
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(LightUBO), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void AddLight(const std::shared_ptr<Light> &l)
    {
        lights.push_back(l);
    }

    void RemoveLight(const std::shared_ptr<Light> &l)
    {
        lights.erase(
            std::remove_if(lights.begin(), lights.end(),
                           [&](const std::weak_ptr<Light> &w)
                           {
                               return w.lock() == l;
                           }),
            lights.end());
    }

    // void CleanupExpiredLights()
    // {
    //     lights.erase(
    //         std::remove_if(lights.begin(), lights.end(),
    //                        [](const std::weak_ptr<Light> &w)
    //                        {
    //                            return w.expired();
    //                        }),
    //         lights.end());
    // }

    void UploadToGPU()
    {
        // 清理过期的灯光
        lights.erase(
            std::remove_if(lights.begin(), lights.end(),
                           [](const std::weak_ptr<Light> &w)
                           {
                               return w.expired();
                           }),
            lights.end());

        int count = std::min((int)lights.size(), MAX_LIGHTS);
        uboData.lightCount = count;

        for (int i = 0; i < count; i++)
        {
            std::shared_ptr<Light> l = lights[i].lock();
            if (!l)
                continue;
            GPULight &g = uboData.lights[i];

            g.type = (int)l->type;
            g.color = l->color;
            g.intensity = l->intensity;

            g.position = l->transform.position();

            if (l->type == LightType::Directional)
            {
                auto dir = std::static_pointer_cast<DirectionalLight>(l);
                g.direction = glm::normalize(dir->direction);
            }
            else if (l->type == LightType::Point)
            {
                auto p = std::static_pointer_cast<PointLight>(l);
                g.range = p->range;
            }
            else if (l->type == LightType::Spot)
            {
                auto s = std::static_pointer_cast<SpotLight>(l);
                g.direction = glm::normalize(s->direction);
                g.range = s->range;
                g.innerCone = s->innerCone;
                g.outerCone = s->outerCone;
            }
        }

        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightUBO), &uboData);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void BindToShader(int bindingPoint)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ubo);
    }
};
