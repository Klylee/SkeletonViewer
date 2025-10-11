#include "Model.h"
#include <iostream>
#include <sstream>
#include <format>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "MeshManager.h"

aiMatrix4x4 GetGlobalTransform(aiNode *node)
{
    aiMatrix4x4 transform = node->mTransformation;
    aiNode *parent = node->mParent;
    while (parent)
    {
        transform = parent->mTransformation * transform;
        parent = parent->mParent;
    }
    return transform;
}

Vector3 GetHead(aiNode *node)
{
    aiMatrix4x4 global = GetGlobalTransform(node);
    aiVector3D origin(0, 0, 0);
    aiVector3D worldPos = global * origin;
    return Vector3(worldPos.x, worldPos.y, worldPos.z);
}

Vector3 GetBoneTailExact(aiNode *node)
{
    aiMatrix4x4 global = GetGlobalTransform(node);

    // 查找尾节点
    aiNode *tailNode = nullptr;
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        std::string childName = node->mChildren[i]->mName.C_Str();
        if (childName.find("_end") != std::string::npos)
        {
            tailNode = node->mChildren[i];
            break;
        }
    }

    if (tailNode)
    {
        aiMatrix4x4 tailGlobal = GetGlobalTransform(tailNode);
        aiVector3D tailPos = tailGlobal * aiVector3D(0, 0, 0);
        return Vector3(tailPos.x, tailPos.y, tailPos.z);
    }
    else
    {
        // 没有尾节点则退化
        aiVector3D defaultTail = global * aiVector3D(0, 0.1f, 0);
        return Vector3(defaultTail.x, defaultTail.y, defaultTail.z);
    }
}

void Model::awake()
{
    Path filepath = directory + filename;
    Assimp::Importer imp;
    const aiScene *scene = imp.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << imp.GetErrorString() << std::endl;
        return;
    }
    processNode(scene->mRootNode, scene);
}

Model::~Model()
{
}

void Model::initialize()
{
    for (int i = 0; i < meshes.size(); i++)
        meshes[i]->initialize();
}

void Model::draw()
{
    for (int i = 0; i < meshes.size(); i++)
    {
        MeshManager::Instance().Submit(meshes[i], material, transform.localToWorld());
        // if (meshes[i]->textures.size() == 0)
        //     shader->setUniform1i("uTextureSample", 0);
        // else
        //     shader->setUniform1i("uTextureSample", 1);
        // meshes[i]->draw(shader);
    }
}

std::string Model::info()
{
    int sumFace = 0;
    std::stringstream ss;
    ss << "mesh number: " << meshes.size() << std::endl;
    for (int i = 0; i < meshes.size(); i++)
    {
        ss << "  mesh" << i << ": " << meshes[i]->v_size / 8 << std::endl;
        sumFace += meshes[i]->v_size / 8;
    }
    ss << "total: " << sumFace << std::endl;
    return ss.str();
}

void Model::printBoneInfo()
{
    std::cout << "Bone Count: " << bones.size() << std::endl;
    for (const auto &[name, head, tail] : bones)
    {
        std::cout << name << ": Head(" << head.x << ", " << head.y << ", " << head.z << ") "
                  << "Tail(" << tail.x << ", " << tail.y << ", " << tail.z << ")" << std::endl;
    }
}

void Model::processNode(aiNode *node, const aiScene *scene)
{
    for (int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(MeshManager::Instance().LoadMesh(mesh, scene, std::format("{}/{}_{}", std::string(directory), filename, meshes.size())));
        for (unsigned int b = 0; b < mesh->mNumBones; ++b)
        {
            aiBone *bone = mesh->mBones[b];
            aiNode *node = scene->mRootNode->FindNode(bone->mName);
            if (!node)
                continue;

            bones.push_back({bone->mName.C_Str(), GetHead(node), GetBoneTailExact(node)});
        }
    }
    for (int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}
