#pragma once
#include <iostream>
#include <memory>
#include <imgui/imgui.h>
#include <config.h>
#include <filesystem>
#include <regex>

#include "App.h"
#include "Path.h"
#include "Event.h"
#include "Model.h"
#include "Material.h"
#include "SceneManager.h"
#include "MeshManager.h"
#include "EventDispatcher.h"

using namespace std::filesystem;

class SkeletonViewerApp : public App
{
public:
    SkeletonViewerApp(int width = 1200, int height = 900, const std::string &title = "SkeletonViewer")
        : App(width, height, title) {}

protected:
    bool InitScene() override
    {
        std::shared_ptr<Shader> shader = std::make_shared<Shader>(std::unordered_map<ShaderVariant, std::string>{
            {ShaderVariant::Basic, Path(ROOT_DIR) + "assets/shader/transparent.shader"},
            {ShaderVariant::Instanced, Path(ROOT_DIR) + "assets/shader/transparent_instanced.shader"}});

        {
            std::shared_ptr<Material> modelMaterial = std::make_shared<Material>(shader);
            modelMaterial->SetUniform("color", "vec4f", glm::vec4(2.0f / 255.0f, 163.0f / 255.0f, 218.0f / 255.0f, 0.3f));
            materials["model"] = modelMaterial;

            std::shared_ptr<Material> nodeMaterial = std::make_shared<Material>(shader);
            nodeMaterial->renderQueue = RenderQueue::Overlay;
            nodeMaterial->renderState.depthFunc = GL_ALWAYS;
            nodeMaterial->renderState.depthWrite = false;
            nodeMaterial->SetUniform("color", "vec4f", glm::vec4(218.0f / 255.0f, 169.0f / 255.0f, 2.0f / 255.0f, 1.0f));
            materials["node"] = nodeMaterial;

            std::shared_ptr<Material> linkMaterial = std::make_shared<Material>(shader);
            linkMaterial->renderQueue = RenderQueue::Overlay;
            linkMaterial->renderState.depthFunc = GL_ALWAYS;
            linkMaterial->renderState.depthWrite = false;
            linkMaterial->SetUniform("color", "vec4f", glm::vec4(113.0f / 255.0f, 121.0f / 255.0f, 224.0f / 255.0f, 1.0f));
            materials["link"] = linkMaterial;
        }

        Event::EventDispatcher::Instance().RegisterHandler<Event::DropEvent>(this, &SkeletonViewerApp::OnDropFiles);
        Event::EventDispatcher::Instance().RegisterHandler<Event::KeyPressedEvent>(this, &SkeletonViewerApp::OnKeyPressed);

        return true;
    }

    void RenderBefore() override
    {
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE)
        {
            jKeyPressed = false;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void RenderAfter() override
    {
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS && !jKeyPressed)
        {
            jKeyPressed = true;
            std::string savedFilename = Path(currentModel).filenameNoExtension() + ".jpg";
            Path savedDir = Path(ROOT_DIR) + "saved";
            if (!savedDir.exist())
            {
                MakeDir(savedDir);
            }
            SaveFrameBuffer(savedDir + savedFilename);
        }
    }

    void RenderImGui() override
    {
        // Docking 区域
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

        // 右侧面板
        ImGui::Begin("Scene Objects");
        ImGui::Text("Drag .obj/.glb files here");
        ImGui::Separator();

        for (int i = 0; i < (int)droppedFiles.size(); i++)
        {
            bool selected = (selectedIndex == i);
            if (ImGui::Selectable(Path(droppedFiles[i]).filename().c_str(), selected))
            {
                selectedIndex = i;

                // 点击时查找对应 SceneObject
                const std::string &objName = Path(droppedFiles[i]).filename();
                auto obj = SceneManager::GetObject<Model>(objName);
                std::cout << "Selected object: " << obj->objName << std::endl;

                if (currentModel != "" && currentModel != objName)
                {
                    auto prevObj = SceneManager::GetObject<Model>(currentModel);
                    if (prevObj)
                    {
                        prevObj->SetActive(false);
                    }
                    obj->SetActive(true);

                    currentModel = objName;
                }
            }
        }
    }

    void OnDropFiles(const std::shared_ptr<Event::DropEvent> &event)
    {
        for (auto &filepath : event->paths)
        {
            AddDroppedFileToScene(filepath);
        }
    }

    void AddDroppedFileToScene(const std::string &filepath)
    {
        Path filepathObj = Path(filepath);
        std::string ext = filepathObj.extension();
        if (ext != "obj" && ext != "glb")
        {
            std::cout << "Only .obj and .glb files are supported." << std::endl;
            return;
        }

        if (SceneManager::GetObject<Model>(filepathObj.filename()))
        {
            std::cout << "Model " << filepathObj.filename() << " already exists in the scene." << std::endl;
            return;
        }

        droppedFiles.push_back(filepathObj.filename());

        if (currentModel == "")
        {
            currentModel = filepathObj.filename();
        }
        else
        {
            auto model = SceneManager::GetObject<Model>(currentModel);
            if (model)
            {
                model->SetActive(false);
            }
            currentModel = filepathObj.filename();
        }

        auto model = std::dynamic_pointer_cast<Model>(SceneObject::create("Model", filepathObj.filename().c_str()));
        SceneManager::AddObject(model);

        // 添加对rignet输入结果的支持，主模型obj，骨骼记录着xxx_rig.txt里面
        if (filepathObj.extension() == "obj")
        {
            auto rig_txt = filepath.substr(0, filepath.size() - 4) + "_noisy.txt";
            std::ifstream file(rig_txt);

            std::unordered_map<std::string, Vector3> jointPositions;
            std::unordered_map<std::string, std::string> parentOf;
            std::unordered_map<std::string, std::vector<std::string>> childrenOf;

            std::regex jointRegex(R"(j\s+<(.+)>\s+(.+)\s+(.+)\s+(.+))");
            std::regex edgeRegex(R"(e\s+<(.+)>\s+<(.+)>)");

            if (file.is_open())
            {
                std::string line;
                while (std::getline(file, line))
                {
                    if (line[0] == 'j')
                    {
                        std::smatch match;
                        if (std::regex_match(line, match, jointRegex))
                        {
                            std::string name = match[1];
                            float x = std::stof(match[2]);
                            float y = std::stof(match[3]);
                            float z = std::stof(match[4]);
                            jointPositions[name] = {x, y, z};
                        }
                    }
                    else if (line[0] == 'e')
                    {
                        std::smatch match;
                        if (std::regex_match(line, match, edgeRegex))
                        {
                            std::string parent = match[1];
                            std::string child = match[2];
                            parentOf[child] = parent;
                            childrenOf[parent].push_back(child);
                        }
                    }
                    // std::istringstream iss(line);
                    // std::string type;
                    // iss >> type;

                    // if (type == "joints")
                    // {
                    //     std::string name;
                    //     float x, y, z;
                    //     iss >> name >> x >> y >> z;
                    //     jointPositions[name] = {x, y, z};
                    // }
                    // else if (type == "hier")
                    // {
                    //     std::string parent, child;
                    //     iss >> parent >> child;
                    //     parentOf[child] = parent;
                    //     childrenOf[parent].push_back(child);
                    // }
                }
                file.close();
            }

            for (const auto &[name, head] : jointPositions)
            {
                Vector3 tail = head; // 默认tail = head
                if (childrenOf.find(name) != childrenOf.end())
                {
                    const auto &children = childrenOf[name];
                    // 若有多个子节点，取平均位置
                    Vector3 avg = {0, 0, 0};
                    for (const auto &c : children)
                    {
                        Vector3 p = jointPositions[c];
                        avg.x += p.x;
                        avg.y += p.y;
                        avg.z += p.z;
                    }
                    float inv = 1.0f / children.size();
                    tail = {avg.x * inv, avg.y * inv, avg.z * inv};
                }

                std::string parent = parentOf.count(name) ? parentOf[name] : "none";
                model->bones[name] = std::make_tuple(head, tail, parent);
            }
        }

        model->directory = filepathObj.directory();
        model->filename = filepathObj.filename();
        model->normalizeMesh = true;
        model->SetMaterial(materials["model"]);
        model->awake();
        model->printBoneInfo();

        model->AddBoneNodes(materials["node"], materials["link"]);

        std::cout << "Added model: " << filepathObj.filename() << std::endl;
    }

    void OnKeyPressed(const std::shared_ptr<Event::KeyPressedEvent> &event)
    {
        if (event->key == GLFW_KEY_P)
        {
            MeshManager::Instance().PrintStatus();
        }
        else if (event->key == GLFW_KEY_DELETE)
        {
            // delete current model
            if (currentModel != "")
            {
                auto model = SceneManager::GetObject<Model>(currentModel);
                if (model)
                {
                    SceneManager::Remove(currentModel);
                }

                droppedFiles.erase(std::remove(droppedFiles.begin(), droppedFiles.end(), currentModel), droppedFiles.end());

                if (!droppedFiles.empty())
                {
                    currentModel = droppedFiles.back();
                    auto model = SceneManager::GetObject<Model>(currentModel);
                    if (model)
                    {
                        model->SetActive(true);
                    }
                }
                else
                {
                    currentModel = "";
                }
            }
        }
    }

    void SaveFrameBuffer(const std::string &filename);

private:
    bool jKeyPressed = false;
    int selectedIndex = -1;
    std::string currentModel = "";
    std::vector<std::string> droppedFiles;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
};
