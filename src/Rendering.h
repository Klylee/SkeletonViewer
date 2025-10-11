#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Shader.h"

struct RenderCommand
{
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Mesh> mesh;
    glm::mat4 modelMatrix;
    // std::shared_ptr<Material> *material;

    // 可选的排序键（预计算好，避免排序时复杂比较）
    uint64_t sortKey;

    // 实例数据（如果使用实例化绘制）
    std::vector<glm::mat4> instances;
};

class RenderQueue
{
public:
    std::vector<RenderCommand> commands;

    void submit(const RenderCommand &cmd)
    {
        commands.push_back(cmd);
    }

    void sort()
    {
        std::sort(commands.begin(), commands.end(),
                  [](const RenderCommand &a, const RenderCommand &b)
                  {
                      return a.sortKey < b.sortKey;
                  });
    }
};
