#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include "Transform.h"

#define REGISTER_SCENE_OBJECT(Derived)                                                        \
    Derived() { className = #Derived; }                                                       \
    struct Derived##Factory                                                                   \
    {                                                                                         \
        Derived##Factory()                                                                    \
        {                                                                                     \
            SceneObject::registry()[#Derived] = []() { return std::make_shared<Derived>(); }; \
        }                                                                                     \
    };                                                                                        \
    static inline Derived##Factory global_##Derived##Factory;

// #define SERIALIZE_FIELDS(...)                        \
//     void Deserialize(const json &j) override         \
//     {                                                \
//         auto tuple = std::tie(__VA_ARGS__);          \
//         deserializeHelper(j, tuple, {#__VA_ARGS__}); \
//     }

// template <typename Tuple>
// void deserializeHelper(const json &j, Tuple &tup, std::vector<std::string> names)
// {
//     for (size_t i = 0; i < names.size(); i++)
//     {
//         if (j.contains(names[i]))
//         {
//             std::apply(
//                 [&](auto &...elems)
//                 {
//                     size_t idx = 0;
//                     ((idx++ == i ? (elems = j[names[i]], 0) : 0), ...);
//                 },
//                 tup);
//         }
//     }
// }

struct SceneObject
{
    std::string className;
    std::string objName;
    Transform transform;
    bool active = true;
    std::vector<std::shared_ptr<SceneObject>> children;

    SceneObject() : transform() {}
    virtual ~SceneObject() = default;

    virtual void awake() {}
    virtual void update() {}
    virtual void draw() {}
    virtual void SetActive(bool isActive)
    {
        active = isActive;
        for (auto &child : children)
        {
            child->SetActive(isActive);
        }
    }

    operator std::string() const { return "<" + className + ">" + objName + "</" + className + ">"; }

    static std::unordered_map<std::string, std::function<std::shared_ptr<SceneObject>()>> &registry()
    {
        static std::unordered_map<std::string, std::function<std::shared_ptr<SceneObject>()>> impl;
        return impl;
    }

    static std::shared_ptr<SceneObject> create(const std::string &_className, const std::string &_objName)
    {
        auto it = registry().find(_className);
        if (it != registry().end())
        {
            auto obj = it->second();
            obj->className = _className;
            obj->objName = _objName;
            return obj;
        }
        return nullptr;
    }
};
