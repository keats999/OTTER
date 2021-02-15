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

class EnemyBehaviour : public IBehaviour
{
public:
	void Update(entt::handle entity) override;
	void SetTarget(entt::handle target) { _target = target.entity(); }

	template <typename T> T LERP(T point0, T point1, float t);

protected:
	entt::entity _target{ entt::null };
	std::vector<glm::vec3> nodes;
	int nodeIndex = 0;
	float timeSpentMoving = 0;
};
