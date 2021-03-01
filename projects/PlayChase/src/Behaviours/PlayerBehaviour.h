#pragma once
#include <SimpleMoveBehaviour.h>
#include <Box2D/box2d.h>
#include "Utilities/Collision2D.h"
#include "Utilities/AudioEngine.h"
#include "GLFW/glfw3.h"
#include "Application.h"
#include "Shader.h"
#include "Transform.h"

class PlayerBehaviour : public SimpleMoveBehaviour
{
public:
	PlayerBehaviour() = default;
	~PlayerBehaviour() = default;

	void Update(entt::handle entity) override;
	void SetShader(Shader::sptr& shader) { _shader = shader; }

private:
	Shader::sptr& _shader = Shader::Create();
	float _maxForwardSpeed = 2;
	float _maxBackwardSpeed = -5;
	float _maxForce = 10;
};
