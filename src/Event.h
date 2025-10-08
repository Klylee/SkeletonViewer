#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "Vec.h"
#include "ParseArg.h"

namespace Event
{

#define REGISTER_EVENT_OBJECT(Derived)                                            \
    struct Derived##Factory                                                       \
    {                                                                             \
        Derived##Factory()                                                        \
        {                                                                         \
            BaseEvent::registry()[#Derived] =                                     \
                [](const std::unordered_map<std::string, std::string> &args) {    \
                    auto obj = Derived::create(args);                             \
                    obj->className = #Derived; return obj; }; \
        }                                                                         \
    };                                                                            \
    static Derived##Factory global_##Derived##Factory;

    enum MouseButton
    {
        Left,
        Mid,
        Right
    };
    enum Keyboard
    {
        A,
        S,
        D,
        F,
        Q,
        W,
        E,
        R,
        Z,
        X,
        C
    };

    struct BaseEvent
    {
        std::string className;

        using Factory = std::function<std::shared_ptr<BaseEvent>(const std::unordered_map<std::string, std::string> &)>;

        static std::unordered_map<std::string, Factory> &registry()
        {
            static std::unordered_map<std::string, Factory> impl;
            return impl;
        }

        static std::shared_ptr<BaseEvent> create(const std::string &name,
                                                 const std::unordered_map<std::string, std::string> &args = {})
        {
            auto &reg = registry();
            auto it = reg.find(name);
            if (it != reg.end())
                return it->second(args);
            return nullptr;
        }
        template <typename T, typename... Args>
        static std::shared_ptr<T> create(Args &&...args)
        {
            return std::make_shared<T>(std::forward<Args>(args)...);
        }
    };
    struct MouseEvent : public BaseEvent
    {
        MouseEvent(float x, float y) : cursor(x, y) {}
        Vec2 cursor;
    };
    struct MouseMoveEvent : public MouseEvent
    {
        REGISTER_EVENT_OBJECT(MouseMoveEvent)
        MouseMoveEvent(float x, float y) : MouseEvent(x, y) {}

        static std::shared_ptr<BaseEvent> create(const std::unordered_map<std::string, std::string> &args)
        {
            double x = getArgAs<double>(args, "x", 0.0);
            double y = getArgAs<double>(args, "y", 0.0);
            return std::make_shared<MouseMoveEvent>(x, y);
        }
    };
    struct MouseButtonEvent : public MouseEvent
    {
        REGISTER_EVENT_OBJECT(MouseButtonEvent)
        enum Action
        {
            Press,
            Release
        };
        MouseButton button;
        Action action;

        MouseButtonEvent(float x, float y, MouseButton _button, Action _action)
            : MouseEvent(x, y), button(_button), action(_action) {}

        static std::shared_ptr<BaseEvent> create(const std::unordered_map<std::string, std::string> &args)
        {
            double x = getArgAs<double>(args, "x", 0.0);
            double y = getArgAs<double>(args, "y", 0.0);
            std::string btn = getArg(args, "button", "Left");
            std::string act = getArg(args, "action", "Press");
            MouseButton button = MouseButton::Left;
            if (btn == "Left")
                button = MouseButton::Left;
            else if (btn == "Mid")
                button = MouseButton::Mid;
            else if (btn == "Right")
                button = MouseButton::Right;
            MouseButtonEvent::Action action = MouseButtonEvent::Press;
            if (act == "Press")
                action = MouseButtonEvent::Press;
            else if (act == "Release")
                action = MouseButtonEvent::Release;
            return std::make_shared<MouseButtonEvent>(x, y, button, action);
        }
    };
    struct MouseScrolledEvent : public MouseEvent
    {
        REGISTER_EVENT_OBJECT(MouseScrolledEvent);
        double xoffset, yoffset;
        MouseScrolledEvent(double x, double y, double xoffset, double yoffset)
            : MouseEvent(x, y), xoffset(xoffset), yoffset(yoffset) {}

        static std::shared_ptr<BaseEvent> create(const std::unordered_map<std::string, std::string> &args)
        {
            double x = getArgAs<double>(args, "x", 0.0);
            double y = getArgAs<double>(args, "y", 0.0);
            double xoffset = getArgAs<double>(args, "xoffset", 0.0);
            double yoffset = getArgAs<double>(args, "yoffset", 0.0);
            return std::make_shared<MouseScrolledEvent>(x, y, xoffset, yoffset);
        }
    };

    struct KeyPressedEvent : public BaseEvent
    {
        REGISTER_EVENT_OBJECT(KeyPressedEvent)
        int key;
        KeyPressedEvent(int _key) : key(_key) {}

        static std::shared_ptr<BaseEvent> create(const std::unordered_map<std::string, std::string> &args)
        {
            int key = getArgAs<int>(args, "key", 0);
            return std::make_shared<KeyPressedEvent>(key);
        }
    };
    struct KeyReleasedEvent : public BaseEvent
    {
        REGISTER_EVENT_OBJECT(KeyReleasedEvent)
        int key;
        KeyReleasedEvent(int _key) : key(_key) {}

        static std::shared_ptr<BaseEvent> create(const std::unordered_map<std::string, std::string> &args)
        {
            int key = getArgAs<int>(args, "key", ' ');
            return std::make_shared<KeyReleasedEvent>(key);
        }
    };

    // record all events generated in every frame
    static std::vector<std::shared_ptr<BaseEvent>> &events()
    {
        static std::vector<std::shared_ptr<BaseEvent>> impl;
        return impl;
    }
}