# pragma once

#include "SceneObject.h"
#include "Mesh.h"

struct Cube : public SceneObject
{
    REGISTER_SCENE_OBJECT(Cube)
    std::shared_ptr<Mesh> mesh;

    void awake() override {}
    void initialize() {}
	void update() override {}
	void draw() override {}
};