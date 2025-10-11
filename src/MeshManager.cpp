#include "MeshManager.h"
#include <iostream>
#include <filesystem>

std::shared_ptr<Mesh> MeshManager::LoadMesh(const std::string &path, const std::string &dict)
{
    namespace fs = std::filesystem;
    std::string absPath = fs::absolute(path).string();

    auto it = meshCache.find(absPath);
    if (it != meshCache.end())
    {
        return it->second; // 已加载过
    }

    // 从文件加载
    auto mesh = std::make_shared<Mesh>(nullptr, nullptr, dict);
    // TODO: 外部的MeshLoader::Load()去解析模型文件
    // mesh = MeshLoader::Load(absPath, dict);

    meshCache[absPath] = mesh;
    return mesh;
}

std::shared_ptr<Mesh> MeshManager::LoadMesh(aiMesh *mesh, const aiScene *scene, const std::string &dict)
{
    // 使用地址哈希或dict作为唯一key
    std::string key = dict; //+ "_" + std::to_string(reinterpret_cast<uintptr_t>(mesh))
    auto it = meshCache.find(key);
    if (it != meshCache.end())
        return it->second;

    auto newMesh = std::make_shared<Mesh>(mesh, scene, dict);
    meshCache[key] = newMesh;
    return newMesh;
}

std::shared_ptr<Mesh> MeshManager::Get(const std::string &key)
{
    auto it = meshCache.find(key);
    if (it != meshCache.end())
        return it->second;
    return nullptr;
}

void MeshManager::Submit(const std::shared_ptr<Mesh> &mesh,
                         const std::shared_ptr<Material> &material,
                         const glm::mat4 &modelMatrix)
{
    BatchKey key{mesh, material};
    batches[key].push_back({modelMatrix});
}

uint64_t MakeSortKey(const std::shared_ptr<Material> &material,
                     const std::shared_ptr<Mesh> &mesh)
{
    auto h1 = reinterpret_cast<uint64_t>(material.get());
    auto h2 = reinterpret_cast<uint64_t>(mesh.get());
    return (h1 << 24) ^ (h2 & 0xFFFFFF);
}

void MeshManager::FlushBatches(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix)
{
    struct SortedBatch
    {
        uint64_t sortKey;
        std::shared_ptr<Material> material;
        std::shared_ptr<Mesh> mesh;
        std::vector<RenderInstance> *instances;
    };
    std::vector<SortedBatch> sortedBatches;
    sortedBatches.reserve(batches.size());
    for (auto &[key, instances] : batches)
    {
        SortedBatch batch;
        batch.material = key.material;
        batch.mesh = key.mesh;
        batch.instances = &instances;
        batch.sortKey = MakeSortKey(key.material, key.mesh);
        sortedBatches.push_back(batch);
    }

    std::sort(sortedBatches.begin(), sortedBatches.end(),
              [](const SortedBatch &a, const SortedBatch &b)
              {
                  return a.sortKey < b.sortKey;
              });

    std::shared_ptr<Shader> currentShader = nullptr;

    for (auto &batch : sortedBatches)
    {
        auto &mesh = batch.mesh;
        auto &material = batch.material;
        auto &instances = batch.instances;

        if (currentShader != material->GetShader())
        {
            currentShader = material->GetShader();
        }

        if (instances->size() > 1)
            currentShader->Use(ShaderVariant::Instanced);
        else
            currentShader->Use(ShaderVariant::Basic);
        currentShader->SetUniformMat4x4f("view", viewMatrix);
        currentShader->SetUniformMat4x4f("projection", projMatrix);

        material->Apply();

        if (instances->size() == 1)
        {
            currentShader->SetUniformMat4x4f("model", instances->at(0).modelMatrix);
            mesh->draw(currentShader);
        }
        else
        {
            // 实例化渲染
            std::vector<glm::mat4> modelMatrices;
            modelMatrices.reserve(instances->size());
            for (auto &inst : *instances)
                modelMatrices.push_back(inst.modelMatrix);
            mesh->drawInstanced(currentShader, modelMatrices);
        }
    }

    batches.clear();
}

void MeshManager::Clear()
{
    meshCache.clear();
    batches.clear();
}

void MeshManager::PrintStatus() const
{
    std::cout << "[MeshManager] Loaded meshes: " << meshCache.size() << std::endl;
    for (auto &[key, val] : meshCache)
    {
        std::cout << " - " << key << std::endl;
    }
}
