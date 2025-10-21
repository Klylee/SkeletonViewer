// #include <GL/glew.h>
// #include <GLFW/glfw3.h>
// #include <string>
// #include <iostream>
// #include <assimp/Importer.hpp>
// #include <assimp/scene.h>
// #include <assimp/postprocess.h>
// #include <chrono>
// #include <thread>
// #include <glm/gtc/type_ptr.hpp>
// #include <imgui/imgui.h>
// #include <backends/imgui_impl_glfw.h>
// #include <backends/imgui_impl_opengl3.h>

// #include "config.h"
// #include "Vec.h"
// #include "Model.h"
// #include "Camera.h"
// #include "Shader.h"
// #include "Path.h"
// #include "Event.h"
// #include "Input.h"
// #include "GlobalTime.h"
// #include "SceneManager.h"
// #include "MeshManager.h"
// #include "EventDispatcher.h"

#include "SkeletonViewerApp.h"

// std::string currentModel = "";
// std::vector<std::string> droppedFiles;
// std::unordered_map<std::string, std::shared_ptr<Material>> materials;

// void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
// {
//     if (action == GLFW_PRESS || action == GLFW_REPEAT)
//     {
//         if (key == GLFW_KEY_P)
//         {
//             MeshManager::Instance().PrintStatus();
//         }
//         else if (key == GLFW_KEY_DELETE)
//         {
//             // delete current model
//             if (currentModel != "")
//             {
//                 auto model = SceneManager::GetObject<Model>(currentModel);
//                 if (model)
//                 {
//                     SceneManager::Remove(currentModel);
//                 }

//                 droppedFiles.erase(std::remove(droppedFiles.begin(), droppedFiles.end(), currentModel), droppedFiles.end());

//                 if (!droppedFiles.empty())
//                 {
//                     currentModel = droppedFiles.back();
//                     auto model = SceneManager::GetObject<Model>(currentModel);
//                     if (model)
//                     {
//                         model->SetActive(true);
//                     }
//                 }
//                 else
//                 {
//                     currentModel = "";
//                 }
//             }
//         }
//         Event::events().push_back(std::make_shared<Event::KeyPressedEvent>(key));
//     }
//     else if (action == GLFW_RELEASE)
//     {
//         Event::events().push_back(std::make_shared<Event::KeyReleasedEvent>(key));
//     }
// }

// void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
// {
//     auto e = std::make_shared<Event::MouseMoveEvent>((float)xpos, (float)ypos);
//     Event::events().push_back(e);
// }

// void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
// {
//     double xpos, ypos;
//     glfwGetCursorPos(window, &xpos, &ypos);

//     Event::MouseButton btn = Event::MouseButton::Left;
//     if (button == GLFW_MOUSE_BUTTON_LEFT)
//         btn = Event::MouseButton::Left;
//     else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
//         btn = Event::MouseButton::Mid;
//     else if (button == GLFW_MOUSE_BUTTON_RIGHT)
//         btn = Event::MouseButton::Right;

//     Event::MouseButtonEvent::Action act = (action == GLFW_PRESS) ? Event::MouseButtonEvent::Press : Event::MouseButtonEvent::Release;

//     auto e = std::make_shared<Event::MouseButtonEvent>((float)xpos, (float)ypos, btn, act);
//     Event::events().push_back(e);
// }

// void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
// {
//     double xpos, ypos;
//     glfwGetCursorPos(window, &xpos, &ypos);

//     auto e = std::make_shared<Event::MouseScrolledEvent>((float)xpos, (float)ypos, xoffset, yoffset);
//     Event::events().push_back(e);
// }

// void AddDroppedFileToScene(const std::string &filepath)
// {
//     Path filepathObj = Path(filepath);
//     std::string ext = filepathObj.extension();
//     if (ext != "obj" && ext != "glb")
//     {
//         std::cout << "Only .obj and .glb files are supported." << std::endl;
//         return;
//     }

//     if (SceneManager::GetObject<Model>(filepathObj.filename()))
//     {
//         std::cout << "Model " << filepathObj.filename() << " already exists in the scene." << std::endl;
//         return;
//     }

//     droppedFiles.push_back(filepathObj.filename());

//     if (currentModel == "")
//     {
//         currentModel = filepathObj.filename();
//     }
//     else
//     {
//         auto model = SceneManager::GetObject<Model>(currentModel);
//         if (model)
//         {
//             model->SetActive(false);
//         }
//         currentModel = filepathObj.filename();
//     }

//     SceneManager::AddObject(SceneObject::create("Model", filepathObj.filename().c_str()));

//     auto model = SceneManager::GetObject<Model>(filepathObj.filename());
//     model->directory = filepathObj.directory();
//     model->filename = filepathObj.filename();
//     model->normalizeMesh = true;
//     model->SetMaterial(materials["model"]);
//     model->awake();
//     model->printBoneInfo();

//     model->AddBoneNodes(materials["node"], materials["link"]);

//     std::cout << "Added model: " << filepathObj.filename() << std::endl;
// }

// void drop_callback(GLFWwindow *window, int count, const char **paths)
// {
//     for (int i = 0; i < count; i++)
//     {
//         AddDroppedFileToScene(paths[i]);
//     }
// }

// #define WIDTH 1200
// #define HEIGHT 900
int main()
{
    std::shared_ptr<SkeletonViewerApp> app = std::make_shared<SkeletonViewerApp>();
    if (!app->Init())
    {
        return -1;
    }

    app->Run();
    app->Destroy();

    return 0;

    // std::cout << "Starting GLFW context, OpenGL 4.6" << std::endl;
    // glfwInit();
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // GLFWwindow *window = glfwCreateWindow(
    //     WIDTH, HEIGHT, "OpenGL Compute Shader Demo", nullptr, nullptr);
    // if (window == nullptr)
    // {
    //     std::cerr << "Failed to create GLFW window" << std::endl;
    //     glfwTerminate();
    //     return -1;
    // }
    // glfwMakeContextCurrent(window);
    // glfwSetKeyCallback(window, key_callback);
    // glfwSetMouseButtonCallback(window, mouse_button_callback);
    // glfwSetCursorPosCallback(window, cursor_position_callback);
    // glfwSetScrollCallback(window, scroll_callback);
    // glfwSetDropCallback(window, drop_callback);

    // // ---------------- ImGui 初始化 ----------------
    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImGuiIO &io = ImGui::GetIO();
    // io.Fonts->AddFontDefault()->Scale = 1.2f;
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // ImGui::StyleColorsDark();
    // ImGui_ImplGlfw_InitForOpenGL(window, true);
    // ImGui_ImplOpenGL3_Init("#version 460");

    // Input::init(window);

    // glewExperimental = GL_TRUE;
    // if (glewInit() != GLEW_OK)
    // {
    //     std::cerr << "Failed to initialize GLEW" << std::endl;
    //     return -1;
    // }

    // std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    // std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    // std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // std::shared_ptr<Shader> shader = std::make_shared<Shader>(std::unordered_map<ShaderVariant, std::string>{
    //     {ShaderVariant::Basic, Path(ROOT_DIR) + "assets/shader/transparent.shader"},
    //     {ShaderVariant::Instanced, Path(ROOT_DIR) + "assets/shader/transparent_instanced.shader"}});

    // {
    //     std::shared_ptr<Material> modelMaterial = std::make_shared<Material>(shader);
    //     modelMaterial->SetUniform("color", "vec4f", glm::vec4(2.0f / 255.0f, 163.0f / 255.0f, 218.0f / 255.0f, 0.3f));
    //     materials["model"] = modelMaterial;

    //     std::shared_ptr<Material> nodeMaterial = std::make_shared<Material>(shader);
    //     nodeMaterial->SetUniform("color", "vec4f", glm::vec4(218.0f / 255.0f, 169.0f / 255.0f, 2.0f / 255.0f, 1.0f));
    //     materials["node"] = nodeMaterial;

    //     std::shared_ptr<Material> linkMaterial = std::make_shared<Material>(shader);
    //     linkMaterial->SetUniform("color", "vec4f", glm::vec4(113.0f / 255.0f, 121.0f / 255.0f, 224.0f / 255.0f, 1.0f));
    //     materials["link"] = linkMaterial;
    // }

    // SceneManager::SetCurrentScene(std::make_shared<Scene>());
    // auto camera = std::dynamic_pointer_cast<Camera>(SceneObject::create("Camera", "main camera"));
    // camera->speed = 0.5f;
    // camera->transform.position(Vector3(0.0f, 0.0f, 1.5f));
    // SceneManager::AddObject(camera);

    // static int selectedIndex = -1;

    // GlobalTime::Init();
    // while (!glfwWindowShouldClose(window))
    // {
    //     glfwPollEvents();

    //     GlobalTime::UpdateLastFrameTime();
    //     GlobalTime::UpdateCurrentFrameTime();
    //     float deltaTime = GlobalTime::GetFrameDeltaTime();
    //     // std::cout << "Delta Time: " << deltaTime << " seconds." << std::endl;

    //     for (auto &e : Event::events())
    //     {
    //         Event::EventDispatcher::Instance().Dispatch(e);
    //     }

    //     SceneManager::Update();

    //     Event::events().clear();

    //     SceneManager::Draw();

    //     glClearColor(1, 1, 1, 1);
    //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //     glEnable(GL_BLEND);
    //     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //     auto camera = SceneManager::GetObject<Camera>("main camera");

    //     MeshManager::Instance().FlushBatches(camera->GetViewMatrix(), camera->GetProjectionMatrix((float)WIDTH / (float)HEIGHT));

    //     MeshManager::Instance().CleanupUnusedMeshes();

    //     // ---------------- ImGui 帧开始 ----------------
    //     ImGui_ImplOpenGL3_NewFrame();
    //     ImGui_ImplGlfw_NewFrame();
    //     ImGui::NewFrame();

    //     // Docking 区域
    //     ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    //     ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

    //     // 右侧面板
    //     ImGui::Begin("Scene Objects");
    //     ImGui::Text("Drag .obj/.glb files here");
    //     ImGui::Separator();

    //     for (int i = 0; i < (int)droppedFiles.size(); i++)
    //     {
    //         bool selected = (selectedIndex == i);
    //         if (ImGui::Selectable(Path(droppedFiles[i]).filename().c_str(), selected))
    //         {
    //             selectedIndex = i;

    //             // 点击时查找对应 SceneObject
    //             const std::string &objName = Path(droppedFiles[i]).filename();
    //             auto obj = SceneManager::GetObject<Model>(objName);
    //             std::cout << "Selected object: " << obj->objName << std::endl;

    //             if (currentModel != "" && currentModel != objName)
    //             {
    //                 auto prevObj = SceneManager::GetObject<Model>(currentModel);
    //                 if (prevObj)
    //                 {
    //                     prevObj->SetActive(false);
    //                 }
    //                 obj->SetActive(true);

    //                 currentModel = objName;
    //             }
    //         }
    //     }

    //     ImGui::End();

    //     // 渲染 ImGui
    //     ImGui::Render();
    //     ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    //     glfwSwapBuffers(window);
    // }

    // shader->Delete();

    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();

    // glfwTerminate();
    // return 0;
}