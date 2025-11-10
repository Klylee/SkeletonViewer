#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "Vec.h"
#include "SceneObject.h"
#include "Path.h"
#include "Material.h"
enum class ObjectType { Mesh, Bone };
class Model : public SceneObject
{
public:
    //添加标签
    ObjectType type = ObjectType::Mesh; // 默认是 Mesh
    REGISTER_SCENE_OBJECT(Model)

    std::shared_ptr<Material> material;
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::unordered_map<std::string, std::tuple<Vector3, Vector3, std::string>> bones; // name-> <head, tail, parentName>
    // 为骨骼增加ID
    std::unordered_map<int, std::string> boneIDs;     // ID -> name
    std::unordered_map<std::string, int> bonenames;   // name -> ID
    std::unordered_map<std::string, int> bone_num;    // name->ID(name形如bone_xx)
    std::unordered_map<int, std::string> bone_num_ID; // ID->name(name形如bone_xx)
    int nextBoneID = 0;
    int numOfBone = 0;

    ~Model() override;

    void awake() override;
    void update() override {}
    void draw() override;

    void SetMaterial(const std::shared_ptr<Material> mat) { material = std::move(mat); }
    std::string info();
    void printBoneInfo();
    void printBoneID();

    // add bone nodes to scene, visualize with nodeMaterial
    void AddBoneNodes(const std::shared_ptr<Material> &nodeMaterial, const std::shared_ptr<Material> &linkMaterial);

    void processNode(aiNode *node, const aiScene *scene);
    Path directory;
    std::string filename;

    bool normalizeMesh = false;
    Vector3 globalCenter = Vector3(0.0f);
    ;
    float globalScale = 1.0f;
};
