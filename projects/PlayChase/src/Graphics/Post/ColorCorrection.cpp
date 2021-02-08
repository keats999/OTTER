#include "ColorCorrection.h"

void ColorCorrection::Init(unsigned width, unsigned height)
{
    int index = int(_buffers.size());
    _buffers.push_back(new Framebuffer());
    _buffers[index]->AddColorTarget(GL_RGBA8);
    _buffers[index]->AddDepthTarget();
    _buffers[index]->Init(width, height);

    index = int(_shaders.size());
    _shaders.push_back(Shader::Create());
    _shaders[index]->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
    _shaders[index]->LoadShaderPartFromFile("shaders/Post/color_correction_frag.glsl", GL_FRAGMENT_SHADER);
    _shaders[index]->Link();

    _testCube.loadFromFile(filename);
    PostEffect::Init(width, height);
 }

void ColorCorrection::ApplyEffect(PostEffect* buffer)
{
    BindShader(0);
    buffer->BindColorAsTexture(0, 0,0);

    _testCube.bind(30);

    _buffers[0]->RenderToFSQ();

    _testCube.unbind(30);

    buffer->UnbindTexture(0);

    UnbindShader();
}


float ColorCorrection::GetIntensity() const
{
    return _intensity;
}

void ColorCorrection::SetIntensity(float intensity)
{
    _intensity = intensity;
}
