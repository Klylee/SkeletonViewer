#include "Model.h"
#include <iostream>
#include <sstream>
#include <format>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "MeshManager.h"
#include "SceneManager.h"

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
    for (auto &b : bones)
    {
        boneIDs[nextBoneID] = filename + "_" + b.first;
        bonenames[filename + "_" + b.first] = nextBoneID;
        bone_num[b.first] = nextBoneID;
        bone_num_ID[nextBoneID] = b.first;
        nextBoneID++;
    }
    numOfBone = nextBoneID;
    printBoneID();

    // if needing to normalize the whole model
    if (normalizeMesh)
    {
        // 第一步：计算所有mesh的全局边界
        float globalMinX = std::numeric_limits<float>::max();
        float globalMaxX = std::numeric_limits<float>::lowest();
        float globalMinY = std::numeric_limits<float>::max();
        float globalMaxY = std::numeric_limits<float>::lowest();
        float globalMinZ = std::numeric_limits<float>::max();
        float globalMaxZ = std::numeric_limits<float>::lowest();

        // 计算所有mesh的全局边界
        for (auto &mesh : meshes)
        {
            float minX = mesh->vertices[0], maxX = mesh->vertices[0];
            float minY = mesh->vertices[1], maxY = mesh->vertices[1];
            float minZ = mesh->vertices[2], maxZ = mesh->vertices[2];

#pragma omp parallel for reduction(min : minX, minY, minZ) reduction(max : maxX, maxY, maxZ)
            for (int i = 1; i < mesh->v_num; i++)
            {
                float x = mesh->vertices[i * 8 + 0];
                float y = mesh->vertices[i * 8 + 1];
                float z = mesh->vertices[i * 8 + 2];

                if (x < minX)
                    minX = x;
                if (x > maxX)
                    maxX = x;
                if (y < minY)
                    minY = y;
                if (y > maxY)
                    maxY = y;
                if (z < minZ)
                    minZ = z;
                if (z > maxZ)
                    maxZ = z;
            }

// 更新全局边界
#pragma omp critical
            {
                if (minX < globalMinX)
                    globalMinX = minX;
                if (maxX > globalMaxX)
                    globalMaxX = maxX;
                if (minY < globalMinY)
                    globalMinY = minY;
                if (maxY > globalMaxY)
                    globalMaxY = maxY;
                if (minZ < globalMinZ)
                    globalMinZ = minZ;
                if (maxZ > globalMaxZ)
                    globalMaxZ = maxZ;
            }
        }

        // 计算全局的中心点和缩放因子
        float globalCenterX = (globalMinX + globalMaxX) / 2.0f;
        float globalCenterY = (globalMinY + globalMaxY) / 2.0f;
        float globalCenterZ = (globalMinZ + globalMaxZ) / 2.0f;
        globalCenter = Vector3(globalCenterX, globalCenterY, globalCenterZ);
        globalScale = std::max({globalMaxX - globalMinX, globalMaxY - globalMinY, globalMaxZ - globalMinZ});

        // 避免除零
        if (globalScale < 1e-6f)
        {
            globalScale = 1.0f;
        }
        else
        {
            globalScale = 1.0f / globalScale;
        }

        // 更改model的transform
        transform.position(-globalCenter * globalScale);
        transform.scale(Vector3(globalScale));
    }
}

Model::~Model()
{
}

void Model::draw()
{
    for (int i = 0; i < meshes.size(); i++)
    {
        MeshManager::Instance().Submit(meshes[i], material, transform.localToWorld());
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

void Model::printBoneID()
{
    for (int i = 0; i < numOfBone; i++)
    {
        std::cout << i << '|' << boneIDs[i] << std::endl;
    }
}

void Model::printBoneInfo()
{
    std::cout << "Bone Count: " << bones.size() << std::endl;
    for (const auto &[name, headAndTail] : bones)
    {
        auto [head, tail, parentName] = headAndTail;
        std::cout << name << ": Head(" << head.x << ", " << head.y << ", " << head.z << ") "
                  << "Tail(" << tail.x << ", " << tail.y << ", " << tail.z << ")"
                  << "Parent: " << parentName << std::endl;
    }
}

void Model::AddBoneNodes(const std::shared_ptr<Material> &nodeMaterial, const std::shared_ptr<Material> &linkMaterial)
{
    for (auto it : bones)
    {
        auto [nodeNmae, headAndTail] = it;
        auto [head, tail, parentName] = headAndTail;
        auto nodeObj = std::dynamic_pointer_cast<Model>(SceneObject::create("Model", filename + "_" + nodeNmae));
        nodeObj->directory = Path(ROOT_DIR) + "/assets";
        nodeObj->filename = "ico-sphere.obj";
        nodeObj->SetMaterial(nodeMaterial);
        nodeObj->awake();
        nodeObj->transform.scale(Vector3(0.01f));
        nodeObj->transform.position((head - globalCenter) * globalScale);
        SceneManager::AddObject(nodeObj);
        children.push_back(nodeObj);

        if (!parentName.empty() && bones.find(parentName) != bones.end())
        {
            auto parentHead = std::get<0>(bones[parentName]);
            auto linkObj = std::dynamic_pointer_cast<Model>(SceneObject::create("Model", filename + "_" + parentName + "-" + nodeNmae));
            linkObj->directory = Path(ROOT_DIR) + "/assets";
            linkObj->filename = "cone.obj";
            linkObj->SetMaterial(linkMaterial);
            linkObj->awake();

            Vector3 direction = glm::normalize(head - parentHead);
            float length = glm::distance(head, parentHead);

            // 计算圆锥体的缩放
            // cone.obj原始高度为2，所以需要缩放为实际骨骼长度的一半
            float heightScale = length / 2.0f;
            // 半径可以根据需要调整，这里设为高度的1/10
            float radiusScale = heightScale * 0.1f;
            glm::vec3 coneScale = glm::vec3(radiusScale, heightScale, radiusScale);
            linkObj->transform.scale(coneScale * globalScale);

            // 计算旋转
            glm::vec3 originalDirection(0.0f, 1.0f, 0.f); // cone.obj原始朝向（高度方向）
            linkObj->transform.rotate(originalDirection, direction);

            // 计算位置
            linkObj->transform.position((parentHead - globalCenter) * globalScale);
            SceneManager::AddObject(linkObj);
            children.push_back(linkObj);
        }
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

            if (node->mParent)
            {
                std::string parentName = node->mParent->mName.C_Str();
                bones[bone->mName.C_Str()] = {GetHead(node), GetBoneTailExact(node), parentName};
            }
            else
            {
                bones[bone->mName.C_Str()] = {GetHead(node), GetBoneTailExact(node), ""};
            }
        }
    }
    for (int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}
