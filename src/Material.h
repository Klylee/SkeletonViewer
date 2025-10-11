#pragma once
#include <unordered_map>
#include <string>
#include <any>
#include <memory>
#include "Shader.h"

class Material
{
    std::shared_ptr<Shader> shader;
    std::unordered_map<std::string, std::pair<std::string, std::any>> uniforms;

public:
    Material(std::shared_ptr<Shader> shader) : shader(std::move(shader)) {}

    void SetUniform(const std::string &name, const std::string &type, const std::any &value)
    {
        uniforms[name] = {type, value};
    }
    std::shared_ptr<Shader> GetShader() const { return shader; }

    void Apply();
};
