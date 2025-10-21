#include "Mesh.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#include <GL/glew.h>
#include <stb_image.h>
#include <iostream>
#include <glm/glm.hpp>

Texture::Texture(const std::string &dict, const std::string &file, TextureType type) : texid(0), type(type)
{
    std::string filepath = dict + "/" + file;
    int w, h, channels;
    unsigned char *img = stbi_load(filepath.c_str(), &w, &h, &channels, 0);
    if (img)
    {
        GLenum format = GL_RED;
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;

        glGenTextures(1, &texid);
        glBindTexture(GL_TEXTURE_2D, texid);
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(img);
    }
    else
    {
        std::cout << "can not load texture file" << std::endl;
    }
}

void Texture::bind(unsigned int channel)
{
    /*glBindTexture(GL_TEXTURE_2D, 0);*/
    glActiveTexture(GL_TEXTURE0 + channel);
    glBindTexture(GL_TEXTURE_2D, texid);
}
void Texture::unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

Mesh::Mesh(aiMesh *mesh, const aiScene *scence, const std::string &dict)
    : vertices(nullptr), indices(nullptr), vao(0), vbo(0), ibo(0), instanceVBO(0)
{
    v_num = mesh->mNumVertices;
    i_num = mesh->mNumFaces * 3;
    v_size = mesh->mNumVertices * 8;
    i_size = mesh->mNumFaces * 3;
    vertices = new float[v_size];
    indices = new unsigned int[i_size];

    for (int i = 0; i < mesh->mNumVertices; i++)
    {
        try
        {
            vertices[i * 8 + 0] = mesh->mVertices[i].x;
            vertices[i * 8 + 1] = mesh->mVertices[i].y;
            vertices[i * 8 + 2] = mesh->mVertices[i].z;
            vertices[i * 8 + 3] = mesh->mNormals[i].x;
            vertices[i * 8 + 4] = mesh->mNormals[i].y;
            vertices[i * 8 + 5] = mesh->mNormals[i].z;
            if (mesh->mTextureCoords[0])
            {
                vertices[i * 8 + 6] = mesh->mTextureCoords[0][i].x;
                vertices[i * 8 + 7] = mesh->mTextureCoords[0][i].y;
            }
            else
            {
                vertices[i * 8 + 6] = 0.0f;
                vertices[i * 8 + 7] = 0.0f;
            }
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_size * sizeof(float), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *)(0 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *)(6 * sizeof(float)));
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
