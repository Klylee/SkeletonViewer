#include "Transform.h"
#include <glm/gtx/quaternion.hpp>

Vector3 Transform::position() const
{
    return _position;
}

void Transform::position(Vector3 pos)
{
    _position = pos;
    calcuLToW();
}

Vector3 Transform::eulerAngles() const
{
    return _eulerAngles;
}

void Transform::eulerAngles(Vector3 eulerangle)
{
    _eulerAngles = eulerangle;
    _rotation = glm::qua<float>(glm::radians(_eulerAngles));
    calcuLToW();
}

Vector3 Transform::scale() const
{
    return _scale;
}

void Transform::scale(Vector3 scale)
{
    _scale = scale;
    calcuLToW();
}

glm::mat4 Transform::localToWorld() const
{
    return _localToWorld;
}

Transform::Transform()
    : _position(0), _eulerAngles(0), _scale(1)
{
    _rotation = glm::qua<float>(glm::radians(_eulerAngles));
    calcuLToW();
}

Quaternion Transform::rotation() const
{
    return _rotation;
}

void Transform::eulerAngles(float yaw, float pitch, float roll)
{
    Vector3 eulerangle(pitch, yaw, roll);
    _eulerAngles = eulerangle;
    _rotation = glm::qua<float>(glm::radians(_eulerAngles));
    calcuLToW();
}

void Transform::rotate(Vector3 originDir, Vector3 targetDir)
{
    _rotation = glm::rotation(originDir, targetDir);
    _eulerAngles = glm::degrees(glm::eulerAngles(_rotation));
    calcuLToW();
}
