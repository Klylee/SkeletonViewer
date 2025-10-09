#pragma once

#ifndef SHADER_H
#define SHADER_H

#include "config.h"
#include <fstream>
#include <unordered_map>
#include <any>

struct ShaderSourceString
{
    std::string vertex_source;
    std::string fragment_source;
};

class Shader
{
    ShaderSourceString PraseShaderSource(const std::string &file);
    unsigned int CompileShader(unsigned int type, const std::string &shader_source);

    int getUnifromLocation(const std::string &label);

public:
    unsigned int program;

    std::unordered_map<std::string, std::pair<std::string, std::any>> uniforms;

    Shader(const std::string &shaderpath);

    void useProgram();
    void deleteProgram();

    void setUniform1f(const std::string &label, float v);
    void setUniform1i(const std::string &label, int v);
    void setUniformVec3f(const std::string &label, float v1, float v2, float v3);
    void setUniformVec3i(const std::string &label, int v1, int v2, int v3);
    void setUniformVec4f(const std::string &label, float v1, float v2, float v3, float v4);
    void setUniformVec4i(const std::string &label, int v1, int v2, int v3, int v4);
    void setUniformMat4x4f(const std::string &label, int matCount, const float *value);

    void setUniforms();
};

#endif // SHADER_H
