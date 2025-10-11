#include "Material.h"

void Material::Apply()
{
    shader->SetUniforms(uniforms);
}