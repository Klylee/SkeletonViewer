#include "SkeletonViewerApp.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void SkeletonViewerApp::SaveFrameBuffer(const std::string &filename)
{
    // 获取窗口尺寸
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // 读取帧缓冲数据
    std::vector<float> pixels(width * height);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, pixels.data());

    float near = 0.01f;
    float far = 0.6f;
    std::vector<float> linearDepth(pixels.size());

    for (size_t i = 0; i < pixels.size(); ++i)
    {
        float z_n = pixels[i] * 2.0f - 1.0f; // NDC [-1, 1]
        linearDepth[i] = (2.0f * near * far) / (far + near - z_n * (far - near));
    }

    std::vector<unsigned char> grayPixels(width * height);
    for (size_t i = 0; i < pixels.size(); ++i)
    {
        // 非线性 depth buffer 映射到 0~255
        grayPixels[i] = static_cast<unsigned char>(linearDepth[i] * 255.0f);
    }

    std::vector<unsigned char> flippedPixels(width * height);
    for (int y = 0; y < height; ++y)
    {
        memcpy(&flippedPixels[y * width],
               &grayPixels[(height - 1 - y) * width],
               width);
    }

    // // 翻转图像（OpenGL的坐标系是左下角为原点）
    // std::vector<unsigned char> flippedPixels(width * height * 3);
    // for (int y = 0; y < height; ++y)
    // {
    //     memcpy(&flippedPixels[y * width * 3],
    //            &pixels[(height - 1 - y) * width * 3],
    //            width * 3);
    // }

    // 保存为JPG
    if (stbi_write_jpg(filename.c_str(), width, height, 1, flippedPixels.data(), 90))
    {
        std::cout << "Screenshot saved: " << filename << std::endl;
    }
    else
    {
        std::cerr << "Failed to save screenshot: " << filename << std::endl;
    }
}