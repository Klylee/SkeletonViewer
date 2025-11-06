#include "Mesh.h"

#include <GL/glew.h>
#include <stb_image.h>
#include <iostream>
#include <glm/glm.hpp>

Mesh::Mesh(aiMesh *mesh, const aiScene *scence, const std::string &dict)
    : vertices(nullptr), indices(nullptr), vao(0), vbo(0), ibo(0), instanceVBO(0)
{
    v_num = mesh->mNumVertices;
    i_num = mesh->mNumFaces * 3;
    v_size = mesh->mNumVertices * 11; // 位置（3）+法线（3）+纹理（2）+颜色（3）
    i_size = mesh->mNumFaces * 3;
    vertices = new float[v_size];
    indices = new unsigned int[i_size];
    printBoneWeights(mesh);
    for (int i = 0; i < mesh->mNumVertices; i++)
    {
        try
        {
            vertices[i * 11 + 0] = mesh->mVertices[i].x;
            vertices[i * 11 + 1] = mesh->mVertices[i].y;
            vertices[i * 11 + 2] = mesh->mVertices[i].z;
            vertices[i * 11 + 3] = mesh->mNormals[i].x;
            vertices[i * 11 + 4] = mesh->mNormals[i].y;
            vertices[i * 11 + 5] = mesh->mNormals[i].z;
            if (mesh->mTextureCoords[0])
            {
                vertices[i * 11 + 6] = mesh->mTextureCoords[0][i].x;
                vertices[i * 11 + 7] = mesh->mTextureCoords[0][i].y;
            }
            else
            {
                vertices[i * 11 + 6] = 0.0f;
                vertices[i * 11 + 7] = 0.0f;
            }
            vertices[i * 11 + 8] = 0.0f;  // R
            vertices[i * 11 + 9] = 0.0f;  // G
            vertices[i * 11 + 10] = 0.0f; // B
        }
        catch (...)
        {
            std::cout << "error happened when initialize vertices" << std::endl;
        }
    }

    for (int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        try
        {
            indices[i * 3 + 0] = face.mIndices[0];
            indices[i * 3 + 1] = face.mIndices[1];
            indices[i * 3 + 2] = face.mIndices[2];
        }
        catch (...)
        {
            std::cout << "error happened when initialize indices" << std::endl;
        }
    }

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *material = scence->mMaterials[mesh->mMaterialIndex];

        for (int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++)
        {
            aiString file;
            material->GetTexture(aiTextureType_DIFFUSE, i, &file);
            std::string texfile = file.C_Str();
            textures.push_back(Texture(dict, texfile, TextureType::DIFFUSE));
        }
        for (int i = 0; i < material->GetTextureCount(aiTextureType_SPECULAR); i++)
        {
            aiString file;
            material->GetTexture(aiTextureType_SPECULAR, i, &file);
            std::string texfile = file.C_Str();
            textures.push_back(Texture(dict, texfile, TextureType::SPECULAR));
        }
        for (int i = 0; i < material->GetTextureCount(aiTextureType_AMBIENT); i++)
        {
            aiString file;
            material->GetTexture(aiTextureType_AMBIENT, i, &file);
            std::string texfile = file.C_Str();
            textures.push_back(Texture(dict, texfile, TextureType::AMBIENT));
        }
    }
    // 将权重数据按骨骼copy一份
    for (unsigned int i = 0; i < mesh->mNumBones; i++)
    {
        aiBone *bone = mesh->mBones[i];
        BoneWeight bw;

        for (unsigned int j = 0; j < bone->mNumWeights; j++)
        {
            bw.vertexIds.push_back(bone->mWeights[j].mVertexId);
            bw.weights.push_back(bone->mWeights[j].mWeight);
        }
        boneWeights[bone->mName.C_Str()] = std::move(bw);
    }
}

void Mesh::printBoneWeights(aiMesh *mesh)
{
    for (unsigned int i = 0; i < mesh->mNumBones; i++)
    {
        aiBone *bone = mesh->mBones[i];
        std::cout << "Bone: " << bone->mName.C_Str() << std::endl;
        for (unsigned int j = 0; j < bone->mNumWeights; j++)
        {
            std::cout << "  Vertex " << bone->mWeights[j].mVertexId
                      << " -> Weight: " << bone->mWeights[j].mWeight << std::endl;
        }
    }
}

Mesh::~Mesh()
{
    if (vertices)
        delete[] vertices;
    if (indices)
        delete[] indices;

    // 释放GPU资源
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &instanceVBO);
}

void Mesh::initialize()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, v_size * sizeof(float), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_size * sizeof(float), indices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_size * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    // 位置
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (const void *)(0 * sizeof(float)));
    // 法线
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (const void *)(3 * sizeof(float)));
    // 纹理
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (const void *)(6 * sizeof(float)));
    // 颜色
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (const void *)(8 * sizeof(float)));
}

void Mesh::draw(std::shared_ptr<Shader> shader)
{
    unsigned int count = 1;
    for (int i = 0; i < textures.size(); i++)
    {
        textures[i].bind(i + 1);
        if (textures[i].type == TextureType::DIFFUSE)
        {
            shader->SetUniform1i(("utexture_diffuse" + std::to_string(count)).c_str(), i + 1);
            count++;
        }
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, i_size, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);

    for (int i = 0; i < textures.size(); i++)
    {
        textures[i].unbind();
    }
}

void Mesh::drawInstanced(const std::shared_ptr<Shader> &shader,
                         const std::vector<glm::mat4> &modelMatrices)
{
    // 如果未创建 instance buffer，则创建
    if (instanceVBO == 0)
    {
        glGenBuffers(1, &instanceVBO);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4),
                     modelMatrices.data(), GL_DYNAMIC_DRAW);

        // 为 mat4 分配 4 个顶点属性位置
        for (int i = 0; i < 4; i++)
        {
            glEnableVertexAttribArray(3 + i);
            glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE,
                                  sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(3 + i, 1);
        }

        glBindVertexArray(0);
    }
    else
    {
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        // 查询当前分配的大小
        GLint currentSize = 0;
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &currentSize);
        GLsizeiptr requiredSize = modelMatrices.size() * sizeof(glm::mat4);

        if (requiredSize > currentSize)
        {
            // 重新分配（容量不够）
            glDeleteBuffers(1, &instanceVBO);
            glGenBuffers(1, &instanceVBO);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferData(GL_ARRAY_BUFFER, requiredSize, modelMatrices.data(), GL_DYNAMIC_DRAW);

            for (int i = 0; i < 4; i++)
            {
                glEnableVertexAttribArray(3 + i);
                glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE,
                                      sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * i));
                glVertexAttribDivisor(3 + i, 1);
            }

            glBindVertexArray(0);
        }
        else
        {
            glBufferSubData(GL_ARRAY_BUFFER, 0, requiredSize, modelMatrices.data());
        }
    }

    glBindVertexArray(vao);
    glDrawElementsInstanced(GL_TRIANGLES, i_size, GL_UNSIGNED_INT, 0, modelMatrices.size());
    glBindVertexArray(0);
}
