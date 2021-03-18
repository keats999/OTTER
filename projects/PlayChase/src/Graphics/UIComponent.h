#pragma once
#include <VertexArrayObject.h>
#include <ShaderMaterial.h>
#include <RendererComponent.h>
#include <ObjLoader.h>

class UIComponent : public RendererComponent{
public:
	VertexArrayObject::sptr Mesh = ObjLoader::LoadFromFile("models/plane.obj");
	ShaderMaterial::sptr    Material;

	UIComponent& SetMesh(const VertexArrayObject::sptr& mesh) { Mesh = mesh;  return *this; }
	UIComponent& SetMaterial(const ShaderMaterial::sptr& material) { Material = material; return *this; }
};