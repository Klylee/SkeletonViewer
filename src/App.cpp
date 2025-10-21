#include "App.h"
#include <iostream>
#include <imgui/imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "Input.h"
#include "Scene.h"
#include "Camera.h"
#include "GlobalTime.h"
#include "MeshManager.h"
#include "SceneManager.h"
#include "EventDispatcher.h"

App::App(int width, int height, const std::string &title)
    : width(width), height(height), title(title)
{
}

App::~App()
{
}

bool App::Init()
{
    if (!InitGLFW())
        return false;
    if (!InitGLEW())
        return false;
    if (!InitImGui())
        return false;

    // 初始化场景与摄像机
    SceneManager::SetCurrentScene(std::make_shared<Scene>());
    auto camera = std::dynamic_pointer_cast<Camera>(SceneObject::create("Camera", "main camera"));
    camera->speed = 0.5f;
    camera->transform.position(Vector3(0.0f, 0.0f, 1.5f));
    SceneManager::SetMainCamera(camera);
    SceneManager::AddObject(camera);

    if (!InitScene())
        return false;

    GlobalTime::Init();
    Input::init(window);
    running = true;

    return true;
}

void App::Destroy()
{
    MeshManager::Instance().Clear();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

bool App::InitGLFW()
{
    std::cout << "Starting GLFW context, OpenGL 4.6" << std::endl;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetDropCallback(window, DropCallback);

    return true;
}

bool App::InitGLEW()
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    return true;
}

bool App::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontDefault()->Scale = 1.2f;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    return true;
}

bool App::InitScene()
{
    return true;
}

void App::Run()
{
    while (running && !glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        GlobalTime::UpdateLastFrameTime();
        GlobalTime::UpdateCurrentFrameTime();
        float deltaTime = GlobalTime::GetFrameDeltaTime();

        ProcessEvents();
        Update();

        RenderBefore();
        RenderClear();
        Render();
        RenderAfter();

        RenderImGuiBefore();
        RenderImGui();
        RenderImGuiAfter();

        glfwSwapBuffers(window);
    }
}

void App::ProcessEvents()
{
    for (auto &e : Event::events())
    {
        Event::EventDispatcher::Instance().Dispatch(e);
    }
    Event::events().clear();
}

void App::Update()
{
    SceneManager::Update();
}

void App::RenderClear()
{
    auto backgroundColor = SceneManager::GetMainCamera()->backgroundColor;
    glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void App::RenderBefore() {}

void App::Render()
{
    SceneManager::Draw(); // 提交绘制

    auto camera = SceneManager::GetMainCamera();

    MeshManager::Instance().FlushBatches(
        camera->GetViewMatrix(),
        camera->GetProjectionMatrix((float)width / (float)height));
    MeshManager::Instance().CleanupUnusedMeshes();
}

void App::RenderAfter() {}

void App::RenderImGuiBefore()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void App::RenderImGui() {}

void App::RenderImGuiAfter()
{
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        Event::events().push_back(std::make_shared<Event::KeyPressedEvent>(key));
    }
    else if (action == GLFW_RELEASE)
    {
        Event::events().push_back(std::make_shared<Event::KeyReleasedEvent>(key));
    }
}

void App::CursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    auto e = std::make_shared<Event::MouseMoveEvent>((float)xpos, (float)ypos);
    Event::events().push_back(e);
}

void App::MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    Event::MouseButton btn = Event::MouseButton::Left;
    if (button == GLFW_MOUSE_BUTTON_LEFT)
        btn = Event::MouseButton::Left;
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        btn = Event::MouseButton::Mid;
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        btn = Event::MouseButton::Right;

    Event::MouseButtonEvent::Action act = (action == GLFW_PRESS) ? Event::MouseButtonEvent::Press : Event::MouseButtonEvent::Release;

    auto e = std::make_shared<Event::MouseButtonEvent>((float)xpos, (float)ypos, btn, act);
    Event::events().push_back(e);
}

void App::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    auto e = std::make_shared<Event::MouseScrolledEvent>((float)xpos, (float)ypos, xoffset, yoffset);
    Event::events().push_back(e);
}

void App::DropCallback(GLFWwindow *window, int count, const char **paths)
{
    std::vector<std::string> pathVec;
    for (int i = 0; i < count; i++)
    {
        pathVec.push_back(paths[i]);
    }
    if (!pathVec.empty())
    {
        auto e = std::make_shared<Event::DropEvent>(pathVec);
        Event::events().push_back(e);
    }
}

void App::OnKeyEvent(int key, int action) {}
void App::OnMouseButtonEvent(int button, int action, double xpos, double ypos) {}
void App::OnCursorPosEvent(double xpos, double ypos) {}
void App::OnScrollEvent(double xoffset, double yoffset, double xpos, double ypos) {}
void App::OnDropEvent(int count, const char **paths) {}
