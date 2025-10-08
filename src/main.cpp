#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <chrono>
#include <thread>
#include <glm/gtc/type_ptr.hpp>

#include "config.h"
#include "Vec.h"
#include "Model.h"
#include "Camera.h"
#include "Shader.h"
#include "Path.h"
#include "Event.h"
#include "Input.h"
#include "GlobalTime.h"

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
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

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    auto e = std::make_shared<Event::MouseMoveEvent>((float)xpos, (float)ypos);
    Event::events().push_back(e);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
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

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    auto e = std::make_shared<Event::MouseScrolledEvent>((float)xpos, (float)ypos, xoffset, yoffset);
    Event::events().push_back(e);
}

#define WIDTH 1200
#define HEIGHT 900
int main()
{
    std::string greeting = "Hello, World!";
    std::cout << greeting << std::endl;
    std::cout << "Starting GLFW context, OpenGL 4.6" << std::endl;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(
        WIDTH, HEIGHT, "OpenGL Compute Shader Demo", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    Input::init(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    std::vector<std::shared_ptr<SceneObject>> sceneObjects;
    std::unordered_map<std::string, std::weak_ptr<SceneObject>> sceneObjectMap;
    {
        auto mainCamera = SceneObject::create("Camera", "main camera");
        auto model = SceneObject::create("Model", "mesh13");
        sceneObjects.push_back(mainCamera);
        sceneObjects.push_back(model);
        // sceneObjects.push_back(SceneObject::create("Model", "node1"));

        sceneObjectMap["main camera"] = mainCamera;
        sceneObjectMap["mesh13"] = model;
    }
    {
        auto model = std::dynamic_pointer_cast<Model>(sceneObjectMap["mesh13"].lock());
        assert(model != nullptr);
        model->directory = Path(ROOT_DIR) + "/assets";
        model->filename = "mesh13_rigged.glb";
        model->awake();
        model->printBoneInfo();
        model->initialize();

        for (auto it : model->bones)
        {
            auto [nodeNmae, head, tail] = it;
            auto nodeObj = std::dynamic_pointer_cast<Model>(SceneObject::create("Model", nodeNmae));
            sceneObjects.push_back(nodeObj);
            nodeObj->directory = Path(ROOT_DIR) + "/assets";
            nodeObj->filename = "ico-sphere.obj";
            nodeObj->awake();
            nodeObj->transform.scale(Vector3(0.0002, 0.0002, 0.0002));
            nodeObj->transform.position(head);
            nodeObj->initialize();
        }
    }

    for (auto it : sceneObjects)
    {
        sceneObjectMap[it->objName] = it;
    }
    {
        // auto node1 = std::dynamic_pointer_cast<Model>(sceneObjectMap["node1"].lock());
        // node1->directory = Path(ROOT_DIR) + "/assets";
        // node1->filename = "ico-sphere.obj";
        // node1->awake();
        // node1->transform.scale(Vector3(0.001, 0.001, 0.001));
        // node1->initialize();

        auto camera = std::dynamic_pointer_cast<Camera>(sceneObjectMap["main camera"].lock());
        assert(camera != nullptr);
        // std::cout << (std::string)*model << (std::string)*camera << std::endl;
    }

    Shader shader(Path(ROOT_DIR) + "assets/shader/transparent.shader");
    shader.useProgram();
    shader.uniforms["color"] = {"vec4f",
                                glm::vec4(2.0f / 255.0f, 163.0f / 255.0f, 218.0f / 255.0f, 0.3f)};

    int ret = glfwWindowShouldClose(window);
    GlobalTime::Init();
    while (!ret)
    {
        Event::events().clear();
        GlobalTime::UpdateLastFrameTime();
        GlobalTime::UpdateCurrentFrameTime();
        float deltaTime = GlobalTime::GetFrameDeltaTime();
        // std::cout << "Delta Time: " << deltaTime << " seconds." << std::endl;

        for (auto it : sceneObjects)
        {
            it->update();
        }

        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        auto camera = std::dynamic_pointer_cast<Camera>(sceneObjectMap["main camera"].lock());
        shader.setUniformMat4x4f("projection", 1, glm::value_ptr(camera->GetProjectionMatrix((float)WIDTH / (float)HEIGHT)));
        shader.setUniformMat4x4f("view", 1, glm::value_ptr(camera->getViewMatrix()));

        auto model = std::dynamic_pointer_cast<Model>(sceneObjectMap["mesh13"].lock());
        shader.setUniformMat4x4f("model", 1, glm::value_ptr(model->transform.localToWorld()));
        shader.uniforms["color"] = {"vec4f",
                                    glm::vec4(2.0f / 255.0f, 163.0f / 255.0f, 218.0f / 255.0f, 0.3f)};
        shader.setUniforms();
        model->draw(std::make_shared<Shader>(shader));

        for (auto it : model->bones)
        {
            auto [nodeNmae, _1, _2] = it;
            auto nodeObj = std::dynamic_pointer_cast<Model>(sceneObjectMap[nodeNmae].lock());
            shader.setUniformMat4x4f("projection", 1, glm::value_ptr(camera->GetProjectionMatrix((float)WIDTH / (float)HEIGHT)));
            shader.setUniformMat4x4f("view", 1, glm::value_ptr(camera->getViewMatrix()));
            shader.setUniformMat4x4f("model", 1, glm::value_ptr(nodeObj->transform.localToWorld()));
            shader.uniforms["color"] = {"vec4f",
                                        glm::vec4(218.0f / 255.0f, 169.0f / 255.0f, 2.0f / 255.0f, 1.0f)};
            shader.setUniforms();
            nodeObj->draw(std::make_shared<Shader>(shader));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}