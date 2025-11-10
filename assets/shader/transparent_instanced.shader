#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;     // 新增：法线输入 (假设在 location 1)
layout (location = 3) in mat4 instanceModel;
layout (location = 7) in vec3 aColor; // 新增：每个顶点的颜色属性（R,G,B）

out vec3 vColor;
out vec3 vNormal;         // 新增：用于光照计算的世界空间法线
out vec3 vWorldPos;       // 新增：用于光照计算的世界空间位置
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = instanceModel * vec4(aPos, 1.0); 
    gl_Position = projection * view * worldPos; // 使用计算好的 worldPos
    vColor = aColor; // 传给片段着色器
    vWorldPos = worldPos.xyz;
    vNormal = mat3(instanceModel) * aNormal;
}

#shader fragment
#version 330 core

in vec3 vColor;
out vec4 FragColor;

uniform vec4 uDefaultColor;       // 原本的 uniform color (重命名，需从 CPU 传入)
uniform vec3 uAmbientColor;       // 环境光颜色 (需从 CPU 传入)
uniform vec3 uLightPos;           // 光源位置 (需从 CPU 传入)
uniform vec3 uLightColor;         // 光源颜色 (需从 CPU 传入)

void main()
{
    // --- 颜色选择与平滑处理 ---
    // 检查经过 GPU 插值后的颜色强度。
    float colorStrength = dot(vColor, vec3(1.0));
    // 如果强度大于阈值（0.01），则说明是权重区域，否则使用默认颜色。
    vec3 baseColor = (colorStrength > 0.01) ? vColor : uDefaultColor.rgb;

    // --- 光照计算 (Ambient + Diffuse) ---
    vec3 norm = normalize(vNormal);

    // 1. 环境光 (Ambient): 确保阴影处不全黑
    vec3 ambient = uAmbientColor * baseColor;

    // 2. 漫反射 (Diffuse): 模拟光线方向
    vec3 lightDir = normalize(uLightPos - vWorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = uLightColor * diff * baseColor;

    // 最终颜色 = 环境光 + 漫反射
    vec3 finalColor = ambient + diffuse;
    
    FragColor = vec4(finalColor, 1.0);
}
