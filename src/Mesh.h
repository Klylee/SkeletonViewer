#pragma once

#include "config.h"
#include "Shader.h"
#include <assimp/scene.h>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Texture.h"

// vertices: n * 8
// pos.x   pos.y   pos.z   nor.x   nor.y   nor.z   tex.u   tex.v
class Mesh
{
public:
    int v_size;
    int i_size;
    int v_num;
    int i_num;
    float *vertices;
    unsigned int *indices;
    std::vector<Texture> textures;

    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;
    unsigned int instanceVBO;

    // 每个骨骼所影响的顶点ID和weight
    struct BoneWeight
    {
        std::vector<int> vertexIds;
        std::vector<float> weights; // 与 vertexIds 对应
    };
    std::unordered_map<std::string, BoneWeight> boneWeights; // key = bonename

    std::vector<glm::vec4> weightColors; // CPU 上每个顶点对应颜色
    unsigned int weightColorVBO = 0;     // GPU buffer
    GLuint weightVAO = 0;                // 新增
    Mesh() : v_size(0), i_size(0), vertices(nullptr), indices(nullptr), vao(0), vbo(0), ibo(0), instanceVBO(0) {}
    Mesh(aiMesh *mesh, const aiScene *scence, const std::string &dict);
    ~Mesh();
    void initialize();
    void draw(std::shared_ptr<Shader> shader);
    void printBoneWeights(aiMesh *mesh);
    void drawInstanced(const std::shared_ptr<Shader> &shader,
                       const std::vector<glm::mat4> &modelMatrices);
};
