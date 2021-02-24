#include "EnemyBehaviour.h"

#include "Application.h"
#include "Timing.h"
#include "Transform.h"
#include "Utilities/MapManager.h"

void EnemyBehaviour::Update(entt::handle entity)
{
	float dt = Timing::Instance().DeltaTime;
	GLFWwindow* window = Application::Instance().Window;

	entt::registry& reg = entity.registry();
	auto targetpos = reg.get<Transform>(_target).GetLocalPosition();

	Transform& transform = entity.get<Transform>();
	
	if (nodeIndex >= nodes.size() - 1 || nodes.size() <= 1)
	{
		nodes = MapManager::Instance().aStar(transform.GetLocalPosition(), targetpos);
		nodeIndex = 0;
	}
	else
	{
		timeSpentMoving += dt / 2;

		if (timeSpentMoving >= 1)
		{
			nodeIndex++;
			timeSpentMoving = 0;
		}

		if (nodeIndex < nodes.size() - 1)
		{
			glm::vec3 oldPos = transform.GetLocalPosition();
			Collision2D& col = entity.get<Collision2D>();
			col.getBody()->SetTransform(b2Vec2(LERP(nodes[nodeIndex], nodes[nodeIndex + 1], timeSpentMoving).x, LERP(nodes[nodeIndex], nodes[nodeIndex + 1], timeSpentMoving).z), col.getBody()->GetAngle());
			glm::vec2 deltaPos = glm::vec2(col.getBody()->GetTransform().p.x - oldPos.x, col.getBody()->GetTransform().p.y - oldPos.z);
			col.getBody()->SetAngle((deltaPos.x > 0.0001f) ? 3.1415928979323f / 2.0f : (deltaPos.x < -0.0001f) ? 3.1415928979323f * 3.0f / 2.0f : (deltaPos.y > 0.0001f) ? 3.1415928979323f : 0.0f);
		}
	}
}

template <typename T>
T EnemyBehaviour::LERP(T point0, T point1, float t)
{
	return (1.0f - t) * point0 + t * point1;
}