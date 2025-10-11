#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Material.h"

class MeshManager
{
public:
    // 获取单例
    static MeshManager &Instance()
    {
        static MeshManager instance;
        return instance;
    }

    // 禁止拷贝
    MeshManager(const MeshManager &) = delete;
    MeshManager &operator=(const MeshManager &) = delete;

    // 通过路径加载或获取已有 Mesh
    std::shared_ptr<Mesh> LoadMesh(const std::string &path, const std::string &dict);

    // 从Assimp直接加载
    std::shared_ptr<Mesh> LoadMesh(aiMesh *mesh, const aiScene *scene, const std::string &dict);

    // 根据唯一key（例如path或hash）获取已存在mesh
    std::shared_ptr<Mesh> Get(const std::string &key);

    // 提交绘制请求（每个 SceneObject 的 Draw 调用时触发）
    void Submit(const std::shared_ptr<Mesh> &mesh,
                const std::shared_ptr<Material> &material,
                const glm::mat4 &modelMatrix);

    // 执行所有批次绘制
    void FlushBatches(const glm::mat4 &viewMatrix, const glm::mat4 &projMatrix);

    // 清除所有资源（例如场景切换）
    void Clear();

    // 打印当前缓存状态
    void PrintStatus() const;

private:
    MeshManager() = default;

    // 用于缓存mesh资源
    std::unordered_map<std::string, std::shared_ptr<Mesh>> meshCache;

    struct RenderInstance
    {
        glm::mat4 modelMatrix;
    };

    struct BatchKey
    {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
        bool operator==(const BatchKey &other) const
        {
            return mesh == other.mesh && material == other.material;
        }
    };

    struct BatchKeyHash
    {
        size_t operator()(const BatchKey &key) const noexcept
        {
            return std::hash<void *>()(key.mesh.get()) ^ (std::hash<void *>()(key.material.get()) << 1);
        }
    };

    std::unordered_map<BatchKey, std::vector<RenderInstance>, BatchKeyHash> batches;
};
