#pragma once
#include "IBehaviour.h"

#include <GLM/glm.hpp>
#include <GLM/gtc/quaternion.hpp>
#include "Camera.h"
#include "Transform.h"
#include <Box2D/box2d.h>
#include "Utilities/Collision2D.h"
#include <windows.h>
#include "Scene.h"

class FirstPersonBehaviour : public IBehaviour
{
public:
	void Update(entt::handle entity) override;
	void SetParent(entt::handle parent) { _parent = parent.entity(); }
	void ToggleMouse() { _trapped = !_trapped; }

protected:
	entt::entity _parent{ entt::null };
	int _mouseX;
	bool _trapped = true;
	float _moveSpeed = 1.5f;
	double _prevMouseX, _prevMouseY;
	float _rotationX, _rotationY;
	bool _isPressed;
	glm::quat _initial;
};
