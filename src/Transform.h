#pragma once

#include <glm/gtc/quaternion.hpp>

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;
typedef glm::mat4 Mat4;
typedef glm::mat3x4 Mat3x4;
typedef glm::qua<float> Quaternion;

class Transform
{
    Vector3 _position;
    Vector3 _eulerAngles;
    Vector3 _scale;
    Quaternion _rotation;
    glm::mat4 _localToWorld;

    void calcuLToW();

public:
    Vector3 position() const;
    void position(Vector3 pos);
    Quaternion rotation() const;
    // angle: deg
    Vector3 eulerAngles() const;

    // 设定旋转角度，不累加，angle: deg
    void eulerAngles(Vector3 eulerangle);

    // 设定旋转角度，不累加，angle: deg
    void eulerAngles(float yaw, float pitch, float roll);

    // 从originDir旋转到targetDir，均为单位向量，不累加
    void rotate(Vector3 originDir, Vector3 targetDir);

    Vector3 scale() const;
    void scale(Vector3 scale);
    glm::mat4 localToWorld() const;

    Transform();
};

// trans * rotate * scale
inline void Transform::calcuLToW()
{
    _localToWorld = glm::mat4(1);
    _localToWorld = glm::translate(_localToWorld, _position);
    _localToWorld = _localToWorld * glm::mat4_cast(_rotation);
    _localToWorld = glm::scale(_localToWorld, _scale);
}