#include "PixelationEffect.h"

void PixelationEffect::Init(unsigned width, unsigned height)
{
    int index = int(_buffers.size());
    _buffers.push_back(new Framebuffer());
    _buffers[index]->AddColorTarget(GL_RGBA8);
    _buffers[index]->Init(width, height);
    
    index = int(_shaders.size());
    _shaders.push_back(Shader::Create());
    _shaders[index]->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
    _shaders[index]->LoadShaderPartFromFile("shaders/Post/pixelation_frag.glsl", GL_FRAGMENT_SHADER);
    _shaders[index]->Link();
}

void PixelationEffect::ApplyEffect(PostEffect* buffer)
{
    BindShader(0);
    _shaders[0]->SetUniform("u_PercentOfPixels", _percentOfPixels);
    buffer->BindColorAsTexture(0, 0, 0);

    _buffers[0]->RenderToFSQ();
    buffer->UnbindTexture(0);
    UnbindShader();
}

void PixelationEffect::DrawToScreen()
{
    BindShader(0);
    _shaders[0]->SetUniform("u_PercentOfPixels", _percentOfPixels);

    BindColorAsTexture(0, 0, 0);

    _buffers[0]->DrawFullscreenQuad();
    
    UnbindTexture(0);

    UnbindShader();
}

float PixelationEffect::GetPercentOfPixels() const
{
    return _percentOfPixels;
}

void PixelationEffect::SetPercentOfPixels(float percentOfPixels)
{
    _percentOfPixels = percentOfPixels;
}
