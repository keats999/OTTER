#include "VignetteEffect.h"

void VignetteEffect::Init(unsigned width, unsigned height)
{
    int index = int(_buffers.size());
    _buffers.push_back(new Framebuffer());
    _buffers[index]->AddColorTarget(GL_RGBA8);
    _buffers[index]->Init(width, height);
    
    index = int(_shaders.size());
    _shaders.push_back(Shader::Create());
    _shaders[index]->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
    _shaders[index]->LoadShaderPartFromFile("shaders/Post/vignette_frag.glsl", GL_FRAGMENT_SHADER);
    _shaders[index]->Link();
}

void VignetteEffect::ApplyEffect(PostEffect* buffer)
{
    BindShader(0);
    _shaders[0]->SetUniform("u_Intensity", _intensity);
    _shaders[0]->SetUniform("u_InnerRadius", _innerRadius);
    _shaders[0]->SetUniform("u_OuterRadius", _outerRadius);
    buffer->BindColorAsTexture(0, 0, 0);

    _buffers[0]->RenderToFSQ();
    buffer->UnbindTexture(0);
    UnbindShader();
}

void VignetteEffect::DrawToScreen()
{
    BindShader(0);
    _shaders[0]->SetUniform("u_Intensity", _intensity);
    _shaders[0]->SetUniform("u_InnerRadius", _innerRadius);
    _shaders[0]->SetUniform("u_OuterRadius", _outerRadius);
    BindColorAsTexture(0, 0, 0);

    _buffers[0]->DrawFullscreenQuad();
    
    UnbindTexture(0);

    UnbindShader();
}

float VignetteEffect::GetIntensity() const
{
    return _intensity;
}

void VignetteEffect::SetIntensity(float intensity)
{
    _intensity = intensity;
}
