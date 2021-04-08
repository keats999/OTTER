#include "CoinBehaviour.h"

#include "Application.h"
#include "Utilities/Globals.h"
#include "Timing.h"
#include "Transform.h"

void CoinBehaviour::Update(entt::handle entity)
{
	if (Application::Instance().ActiveScene == Globals::Instance().scenes[1])
	{
		float dt = Timing::Instance().DeltaTime;
		entity.get<Transform>().RotateLocal(_speed * dt, 0, 0);
	}
}