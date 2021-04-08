#pragma once
#include "IBehaviour.h"

#include <GLM/glm.hpp>
#include <GLM/gtc/quaternion.hpp>
#include "RendererComponent.h"
#include "ObjLoader.h"
#include "Transform.h"
#include <Box2D/box2d.h>
#include "Utilities/Collision2D.h"
#include <windows.h>
#include "Scene.h"

class ExitBehaviour : public IBehaviour
{
public:
	void Update(entt::handle entity) override;
protected:
	VertexArrayObject::sptr newtube = ObjLoader::LoadFromFile("models/tubesc.obj");
};
