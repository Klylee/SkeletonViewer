#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 3) in mat4 instanceModel;
layout (location = 7) in vec3 aColor; // 新增：每个顶点的颜色属性（R,G,B）

out vec3 vColor;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * instanceModel * vec4(aPos, 1.0);
    vColor = aColor; // 传给片段着色器
}

#shader fragment
#version 330 core

in vec3 vColor;
out vec4 FragColor;

uniform vec4 color; // 原始 uniform 颜色（淡蓝）

void main()
{
    // 如果顶点颜色几乎是黑色（0,0,0），说明没有被设置，就用默认淡蓝色
    if (length(vColor) == 0.00)
        FragColor = color;
    else
        FragColor = vec4(vColor, 1.0);
}
