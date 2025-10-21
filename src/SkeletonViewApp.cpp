#include "SkeletonViewerApp.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void SkeletonViewerApp::SaveFrameBuffer(const std::string &filename)
{
    // 获取窗口尺寸
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // 读取帧缓冲数据
    std::vector<unsigned char> pixels(width * height * 3);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // 翻转图像（OpenGL的坐标系是左下角为原点）
    std::vector<unsigned char> flippedPixels(width * height * 3);
    for (int y = 0; y < height; ++y)
    {
        memcpy(&flippedPixels[y * width * 3],
               &pixels[(height - 1 - y) * width * 3],
               width * 3);
    }

    // 保存为JPG
    if (stbi_write_jpg(filename.c_str(), width, height, 3, flippedPixels.data(), 90))
    {
        std::cout << "Screenshot saved: " << filename << std::endl;
    }
    else
    {
        std::cerr << "Failed to save screenshot: " << filename << std::endl;
    }
}