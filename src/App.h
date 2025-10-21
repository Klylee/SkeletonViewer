#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

class App
{
public:
    App(int width = 1200, int height = 900, const std::string &title = "OpenGL Application");
    virtual ~App();

    bool Init();
    void Run();
    void Destroy();

    GLFWwindow *GetWindow() const { return window; }
    int Width() const { return width; }
    int Height() const { return height; }

protected:
    virtual bool InitGLFW();
    virtual bool InitGLEW();
    virtual bool InitImGui();

    // 用于初始化场景资源，例如加载模型、材质，着色器等
    virtual bool InitScene();

    virtual void ProcessEvents();
    virtual void Update();
    virtual void RenderBefore();
    virtual void RenderClear();
    virtual void Render();
    virtual void RenderAfter();
    virtual void RenderImGuiBefore();
    virtual void RenderImGui();
    virtual void RenderImGuiAfter();

    virtual void OnKeyEvent(int key, int action);
    virtual void OnMouseButtonEvent(int button, int action, double xpos, double ypos);
    virtual void OnCursorPosEvent(double xpos, double ypos);
    virtual void OnScrollEvent(double xoffset, double yoffset, double xpos, double ypos);
    virtual void OnDropEvent(int count, const char **paths);

    GLFWwindow *window = nullptr;
    int width;
    int height;
    std::string title;
    bool running = false;

private:
    static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow *window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void DropCallback(GLFWwindow *window, int count, const char **paths);
};