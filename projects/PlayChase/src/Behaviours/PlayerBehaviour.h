#pragma once
#include <SimpleMoveBehaviour.h>
#include <Box2D/box2d.h>
#include "Utilities/Collision2D.h"
#include "Utilities/AudioEngine.h"
#include "GLFW/glfw3.h"
#include "Application.h"

class PlayerBehaviour : public SimpleMoveBehaviour
{
public:
	PlayerBehaviour() = default;
	~PlayerBehaviour() = default;

	void Update(entt::handle entity) override;

private:
	float _maxForwardSpeed = 5;
	float _maxBackwardSpeed = -3;
	float _maxForce = 10;
};
