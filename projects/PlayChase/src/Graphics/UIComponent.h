#pragma once
#include <VertexArrayObject.h>
#include <ShaderMaterial.h>
#include <RendererComponent.h>
#include <ObjLoader.h>

class UIComponent : public RendererComponent{
public:
	UIComponent& SetMesh(const VertexArrayObject::sptr& mesh = ObjLoader::LoadFromFile("models/plane.obj")) { Mesh = mesh;  return *this; }
};