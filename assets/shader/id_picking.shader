
#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
//layout (location = 3) in mat4 instanceModel;
layout (location = 7) in vec3 aColor; // 新增：顶点颜色（R,G,B）

out vec3 vColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vColor = aColor;
}

#shader fragment
#version 330 core

in vec3 vColor;
out vec4 FragColor;

uniform vec4 color;  // 原本的淡蓝色

void main()
{
    // 如果顶点颜色几乎为零，就使用 uniform 颜色；否则使用顶点颜色
    FragColor = color;  // 默认的淡蓝色
}