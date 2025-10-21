#pragma once
#include <memory>
#include "Scene.h"

class SceneManager
{
public:
    static void SetCurrentScene(const std::shared_ptr<Scene> &scene)
    {
        currentScene = scene;
    }

    static std::shared_ptr<Scene> GetCurrentScene()
    {
        return currentScene;
    }

    // 以下封装直接代理给 currentScene
    static void AddObject(const std::shared_ptr<SceneObject> &obj)
    {
        if (currentScene)
            currentScene->AddSceneObject(obj);
    }

    template <typename T>
    static std::shared_ptr<T> GetObject(const std::string &name)
    {
        if (!currentScene)
            return nullptr;
        return currentScene->GetObject<T>(name);
    }

    static void Update()
    {
        if (currentScene)
            currentScene->UpdateAll();
    }

    static void Draw()
    {
        if (currentScene)
            currentScene->DrawAll();
    }

    static void Remove(const std::string &name)
    {
        if (currentScene)
            currentScene->Remove(name);
    }

    static std::shared_ptr<Camera> GetMainCamera()
    {
        if (currentScene)
            return currentScene->GetMainCamera();
        return nullptr;
    }

    static void SetMainCamera(const std::shared_ptr<Camera> &camera)
    {
        if (currentScene)
            currentScene->SetMainCamera(camera);
    }

private:
    inline static std::shared_ptr<Scene> currentScene = nullptr;
};
