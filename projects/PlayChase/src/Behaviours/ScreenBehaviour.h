#pragma once
#include "IBehaviour.h"
#include <GLM/glm.hpp>
#include "Timing.h"
#include <Texture2D.h>
#include <Texture2DData.h>
#include "Transform.h"
#include "GLFW/glfw3.h"
#include "Application.h"
#include "RendererComponent.h"
#include <memory>

class ScreenBehaviour : public IBehaviour
{
public:
	void Update(entt::handle entity) override;
	void SetMaterial(ShaderMaterial::sptr mat);
	bool playercontact = false;
private:
	bool pressed = false;
	float presstimer = 0.0f;
	std::vector<Texture2D::sptr> screens;
	ShaderMaterial::sptr& screenMat = ShaderMaterial::Create();
	
};

