#pragma once
#include <unordered_map>
#include <string>
#include <any>
#include <memory>
#include <GL/glew.h>

#include "Shader.h"

struct RenderState
{
    bool depthTest = true;
    bool depthWrite = true;
    bool blend = false;
    bool cullFace = false;
    GLenum depthFunc = GL_LEQUAL;
    GLenum blendSrc = GL_SRC_ALPHA;
    GLenum blendDst = GL_ONE_MINUS_SRC_ALPHA;
};

enum RenderQueue
{
    Background = 1000,
    Geometry = 2000,
    AlphaTest = 2450,
    Transparent = 3000,
    Overlay = 4000
};

class Material
{
    std::shared_ptr<Shader> shader;
    std::unordered_map<std::string, std::pair<std::string, std::any>> uniforms;

public:
    RenderState renderState;
    unsigned int renderQueue = RenderQueue::Geometry;

    Material(std::shared_ptr<Shader> shader) : shader(std::move(shader)) {}

    void SetUniform(const std::string &name, const std::string &type, const std::any &value)
    {
        uniforms[name] = {type, value};
    }
    std::shared_ptr<Shader> GetShader() const { return shader; }

    void ApplyUniforms();
    void ApplyRenderState();
};
