#pragma once

#include "config.h"
#include "Shader.h"
#include <assimp/scene.h>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

enum TextureType
{
    DIFFUSE,
    SPECULAR,
    AMBIENT
};
class Texture
{
public:
    unsigned int texid;
    TextureType type;

    Texture() = default;
    Texture(const std::string &dict, const std::string &file, TextureType type);
    void bind(unsigned int channel);
    void unbind();
};
// vertices: n * 8
// pos.x   pos.y   pos.z   nor.x   nor.y   nor.z   tex.u   tex.v

class Mesh
{
public:
    int v_size;
    int i_size;
    float *vertices;
    unsigned int *indices;
    std::vector<Texture> textures;

    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;
    unsigned int instanceVBO;

    Mesh() : v_size(0), i_size(0), vertices(nullptr), indices(nullptr), vao(0), vbo(0), ibo(0), instanceVBO(0) {}
    Mesh(aiMesh *mesh, const aiScene *scence, const std::string &dict);
    ~Mesh();
    void initialize();
    void draw(std::shared_ptr<Shader> shader);

    void drawInstanced(const std::shared_ptr<Shader> &shader,
                       const std::vector<glm::mat4> &modelMatrices);
};
