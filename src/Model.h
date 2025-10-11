#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "Vec.h"
#include "SceneObject.h"
#include "Path.h"
#include "Material.h"

class Model : public SceneObject
{
public:
    REGISTER_SCENE_OBJECT(Model)

    std::shared_ptr<Material> material;
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::tuple<std::string, Vector3, Vector3>> bones; // name, head, tail

    ~Model() override;

    void initialize();

    void awake() override;
    void update() override {}
    void draw() override;

    void SetMaterial(const std::shared_ptr<Material> mat) { material = std::move(mat); }
    std::string info();
    void printBoneInfo();

    void processNode(aiNode *node, const aiScene *scene);
    Path directory;
    std::string filename;
};
