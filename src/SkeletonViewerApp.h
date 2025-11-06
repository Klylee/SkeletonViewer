#pragma once
#include <iostream>
#include <memory>
#include <imgui/imgui.h>
#include <config.h>
#include <filesystem>

#include "App.h"
#include "Path.h"
#include "Event.h"
#include "Model.h"
#include "Material.h"
#include "SceneManager.h"
#include "MeshManager.h"
#include "EventDispatcher.h"
#include "Camera.h"
#include "mesh.h"
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
            nodeMaterial->SetUniform("color", "vec4f", glm::vec4(218.0f / 255.0f, 169.0f / 255.0f, 2.0f / 255.0f, 1.0f));
            materials["node"] = nodeMaterial;

            std::shared_ptr<Material> linkMaterial = std::make_shared<Material>(shader);
            linkMaterial->SetUniform("color", "vec4f", glm::vec4(113.0f / 255.0f, 121.0f / 255.0f, 224.0f / 255.0f, 1.0f));
            materials["link"] = linkMaterial;
        }
        // std::cout << "[DBG] currentModel=" << currentModel << std::endl;
        // 初始化id为0到100对应的材质颜色：
        {
            idMaterials.resize(100);
            for (int i = 0; i < 100; ++i)
            {
                float gray = i;                 // 灰度值 0~100
                float n = gray / 100.0f;        // 映射到 0~1
                glm::vec4 color(n, n, n, 1.0f); // vec4(R=G=B=gray, A=1)

                idMaterials[i] = std::make_shared<Material>(shader);
                idMaterials[i]->SetUniform("color", "vec4f", color); // 注意 uniform 名称和 shader 匹配
            }
        }
        Event::EventDispatcher::Instance().RegisterHandler<Event::DropEvent>(this, &SkeletonViewerApp::OnDropFiles);
        Event::EventDispatcher::Instance().RegisterHandler<Event::KeyPressedEvent>(this, &SkeletonViewerApp::OnKeyPressed);
        Event::EventDispatcher::Instance().RegisterHandler<Event::MouseButtonEvent>(this, &SkeletonViewerApp::OnMouseButton);

        return true;
    }

    void RenderBefore() override
    {
        auto backgroundColor = SceneManager::GetMainCamera()->backgroundColor;
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE)
        {
            jKeyPressed = false;
        }
        if (mouseClickPending && !currentModel.empty())
        {
            // --- 保存当前状态 ---
            GLboolean prevDepthTest = glIsEnabled(GL_DEPTH_TEST);
            GLboolean prevBlend = glIsEnabled(GL_BLEND);
            GLint prevDepthMask;
            glGetIntegerv(GL_DEPTH_WRITEMASK, &prevDepthMask);

            // --- 为 ID pass 设置确定的状态 ---
            // 不希望 blend 影响 ID 颜色：禁用 blend
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE); // 不写入深度缓冲
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            auto obj = SceneManager::GetObject<Model>(currentModel);
            auto shader = idMaterials[0]->GetShader(); // 共享 shader
            shader->Use(ShaderVariant::Basic);
            auto camera = SceneManager::GetMainCamera();
            for (auto &child : obj->children)
            {
                if (auto modelChild = std::dynamic_pointer_cast<Model>(child))
                {
                    // 通过名字查ID
                    auto it = obj->bonenames.find(modelChild->objName);
                    if (it == obj->bonenames.end())
                    {
                        // 不在 map 中，说明是关节或者非骨骼节点，跳过
                        continue;
                    }

                    modelChild = std::dynamic_pointer_cast<Model>(child);
                    int id = obj->bonenames[modelChild->objName];
                    // int id = it->second; // 安全获取 ID
                    auto mat = idMaterials[id];
                    // 设置材质
                    mat->Apply();
                    // 给每个骨骼节点设置对应的颜色材质
                    modelChild->SetMaterial(mat);
                    // shader->SetUniformVec4f("color", glm::vec4(1,0,0,1));
                    modelChild->draw();
                }
            }
            MeshManager::Instance().FlushBatches(
                camera->GetViewMatrix(),
                camera->GetProjectionMatrix((float)width / (float)height));
            MeshManager::Instance().CleanupUnusedMeshes();
            glFinish();
            // 读取像素
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
            double ypos = fbHeight - mouseY; // 翻转Y
            unsigned char pixel[4];
            glReadPixels((int)mouseX, (int)ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

            int id = int((pixel[0] / 255.0f) * 100.0f + 0.5f);
            selectedBoneName = obj->bone_num_ID[id];
            std::cout << "Clicked bone: " << selectedBoneName << " (ID=" << id << ")" << std::endl;

            mouseClickPending = false; // 重置点击
            for (auto &child : obj->children)
            {
                if (auto modelChild = std::dynamic_pointer_cast<Model>(child))
                {
                    modelChild->SetMaterial(materials["node"]); // 恢复彩色材质
                }
            }
            // 恢复写深度（如果之前是写深度的）
            if (prevDepthMask)
                glDepthMask(GL_TRUE);
            else
                glDepthMask(GL_FALSE);

            // 恢复 depth test 状态
            if (!prevDepthTest)
                glDisable(GL_DEPTH_TEST); // 如果之前没启用则禁用
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    void Render() override
    {
        // 增加权重颜色
        if (selectedBoneName != "")
        {
            if (currentModel.empty())
                return;

            auto model = SceneManager::GetObject<Model>(currentModel);
            if (!model)
                return;

            // 遍历模型的每个 Mesh
            for (auto &mesh : model->meshes)
            {
                glm::vec3 defaultBlue(2.0f / 255.0f, 163.0f / 255.0f, 218.0f / 255.0f);
                for (int i = 0; i < mesh->v_num; i++)
                {
                    mesh->vertices[i * 11 + 8] = defaultBlue.r;
                    mesh->vertices[i * 11 + 9] = defaultBlue.g;
                    mesh->vertices[i * 11 + 10] = defaultBlue.b;
                }
                auto it = mesh->boneWeights.find(selectedBoneName);
                if (it != mesh->boneWeights.end())
                {
                    const auto &bw = it->second;
                    for (size_t i = 0; i < bw.vertexIds.size(); ++i)
                    {
                        int vid = bw.vertexIds[i];
                        float weight = bw.weights[i]; // 0~1

                        weight = glm::clamp(weight, 0.0f, 1.0f);
                        // 从浅蓝 -> 红 渐变
                        glm::vec3 blue = glm::vec3(2.0f / 255.0f, 163.0f / 255.0f, 218.0f / 255.0f);
                        glm::vec3 red = glm::vec3(1.0f, 0.0f, 0.0f);
                        glm::vec3 color = glm::mix(blue, red, weight);

                        mesh->vertices[vid * 11 + 8] = color.r;
                        mesh->vertices[vid * 11 + 9] = color.g;
                        mesh->vertices[vid * 11 + 10] = color.b;
                    }
                    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->v_size * sizeof(float), mesh->vertices);
                }
            }
        }
        // 渲染
        SceneManager::Draw();
        auto camera = SceneManager::GetMainCamera();
        MeshManager::Instance().FlushBatches(camera->GetViewMatrix(),
                                             camera->GetProjectionMatrix((float)width / (float)height));
        MeshManager::Instance().CleanupUnusedMeshes();
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
                        for (auto &mesh : prevObj->meshes)
                        {
                            // CPU 内存清空
                            mesh->weightColors.clear();

                            // GPU buffer 删除
                            if (mesh->weightColorVBO != 0)
                            {
                                glDeleteBuffers(1, &mesh->weightColorVBO);
                                mesh->weightColorVBO = 0;
                            }

                            // 如果 DrawWithBoneWeight 内部有 flag 表示使用权重渲染
                            // mesh->useWeightColor = false;
                        }

                        // 切换模型的时候释放材质
                        for (auto &child : prevObj->children)
                        {
                            if (auto childModel = std::dynamic_pointer_cast<Model>(child))
                                childModel->material.reset();
                        }
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
            auto rig_txt = filepath.substr(0, filepath.size() - 4) + "_rig.txt";
            std::ifstream file(rig_txt);

            std::unordered_map<std::string, Vector3> jointPositions;
            std::unordered_map<std::string, std::string> parentOf;
            std::unordered_map<std::string, std::vector<std::string>> childrenOf;

            if (file.is_open())
            {
                std::string line;
                while (std::getline(file, line))
                {
                    std::istringstream iss(line);
                    std::string type;
                    iss >> type;

                    if (type == "joints")
                    {
                        std::string name;
                        float x, y, z;
                        iss >> name >> x >> y >> z;
                        jointPositions[name] = {x, y, z};
                    }
                    else if (type == "hier")
                    {
                        std::string parent, child;
                        iss >> parent >> child;
                        parentOf[child] = parent;
                        childrenOf[parent].push_back(child);
                    }
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
        // model->printBoneID();

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
    void OnMouseButton(const std::shared_ptr<Event::MouseButtonEvent> &event)
    {
        if (event->action == Event::MouseButtonEvent::Press &&
            event->button == Event::MouseButton::Left)
        {
            mouseClickPending = true;
            mouseX = event->cursorX;
            mouseY = event->cursorY;
            std::cout << "Left mouse clicked at (" << mouseX << ", " << mouseY << ")" << std::endl;
        }
    }

private:
    bool jKeyPressed = false;
    int selectedIndex = -1;
    std::string selectedBoneName = "";
    std::string currentModel = "";
    bool mouseClickPending = false; // 点击标志
    float mouseX, mouseY;           // 鼠标位置
    std::vector<std::string> droppedFiles;
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::vector<std::shared_ptr<Material>> idMaterials;
    std::shared_ptr<Shader> weightShader;
};
