#pragma once
#include <VertexArrayObject.h>
#include <ShaderMaterial.h>
#include <RendererComponent.h>
#include <ObjLoader.h>

class UIComponent : public RendererComponent{
public:
	VertexArrayObject::sptr Mesh;
	ShaderMaterial::sptr    Material;

	UIComponent& SetMesh() { Mesh = ObjLoader::LoadFromFile("models/plane.obj");  return *this; }
	UIComponent& SetMaterial(const ShaderMaterial::sptr& material) { Material = material; return *this; }
};