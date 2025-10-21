#pragma once

#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#include "Camera.h"
#include "SceneObject.h"

class Scene
{
public:
    // 添加对象（注册进场景）
    void AddSceneObject(const std::shared_ptr<SceneObject> &obj)
    {
        if (!obj)
            return;

        if (sceneObjectMap.find(obj->objName) != sceneObjectMap.end())
            return;

        sceneObjects.push_back(obj);
        sceneObjectMap[obj->objName] = obj;
        std::cout << "Added " << "<" << obj->className << ">" << obj->objName << std::endl;
    }

    // 通过名字查找对象
    template <typename T>
    std::shared_ptr<T> GetObject(const std::string &name)
    {
        static_assert(std::is_base_of_v<SceneObject, T>, "T must derive from SceneObject");

        auto it = sceneObjectMap.find(name);
        if (it == sceneObjectMap.end())
            return nullptr;

        auto basePtr = it->second.lock();
        if (!basePtr)
            return nullptr;

        return std::dynamic_pointer_cast<T>(basePtr);
    }

    // 移除对象和它的子对象
    void Remove(const std::string &name)
    {
        auto it = sceneObjectMap.find(name);
        if (it != sceneObjectMap.end())
        {
            auto target = it->second.lock();
            if (target)
            {
                for (auto &child : target->children)
                {
                    Remove(child->objName);
                }
                sceneObjects.erase(
                    std::remove_if(sceneObjects.begin(), sceneObjects.end(),
                                   [&](auto &o)
                                   { return o == target; }),
                    sceneObjects.end());
            }
            sceneObjectMap.erase(it);
            std::cout << "Removed " << "<" << target->className << ">" << name << std::endl;
        }
    }

    // 调用所有对象的更新与绘制
    void UpdateAll()
    {
        for (auto &obj : sceneObjects)
        {
            if (obj && obj->active)
                obj->update();
        }
    }

    void DrawAll()
    {
        for (auto &obj : sceneObjects)
        {
            if (obj && obj->active)
                obj->draw();
        }
    }

    // 获取所有对象
    const std::vector<std::shared_ptr<SceneObject>> &GetObjects() const { return sceneObjects; }

    // 设置主相机
    void SetMainCamera(const std::shared_ptr<Camera> &camera)
    {
        mainCamera = camera;
    }

    std::shared_ptr<Camera> GetMainCamera() const
    {
        return mainCamera;
    }

private:
    std::vector<std::shared_ptr<SceneObject>> sceneObjects;
    std::unordered_map<std::string, std::weak_ptr<SceneObject>> sceneObjectMap;
    std::shared_ptr<Camera> mainCamera;
};
