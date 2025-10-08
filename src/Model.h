#pragma once

#include "Mesh.h"
#include "Vec.h"
#include "SceneObject.h"
#include <glm/glm.hpp>
#include "Path.h"

class Model : public SceneObject
{
public:
    REGISTER_SCENE_OBJECT(Model)

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::tuple<std::string, Vector3, Vector3>> bones; // name, head, tail

    ~Model() override;

    void initialize();

    void awake() override;
    void update() override {}
    void draw(std::shared_ptr<Shader> shader);
    std::string info();
    void printBoneInfo();

    void processNode(aiNode *node, const aiScene *scene);
    Path directory;
    std::string filename;
};
