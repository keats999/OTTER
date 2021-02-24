#pragma once
#include "IBehaviour.h"

#include <GLM/glm.hpp>
#include <GLM/gtc/quaternion.hpp>
#include "Camera.h"
#include "Transform.h"
#include <windows.h>
#include "Scene.h"

class CoinBehaviour : public IBehaviour
{
public:
	void Update(entt::handle entity) override;

protected:
	float _speed = 10;
};
