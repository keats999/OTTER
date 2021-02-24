#include "CoinBehaviour.h"

#include "Application.h"
#include "Timing.h"
#include "Transform.h"

void CoinBehaviour::Update(entt::handle entity)
{
	float dt = Timing::Instance().DeltaTime;
	entity.get<Transform>().RotateLocal(_speed * dt, 0, 0);
}