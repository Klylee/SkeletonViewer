#include "Material.h"
#include <GL/glew.h>

void Material::ApplyUniforms()
{
    shader->SetUniforms(uniforms);
}

void Material::ApplyRenderState()
{
    if (renderState.depthTest)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);

    glDepthMask(renderState.depthWrite ? GL_TRUE : GL_FALSE);

    if (renderState.blend)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

    if (renderState.cullFace)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    glBlendFunc(renderState.blendSrc, renderState.blendDst);
}
