#pragma once
#include "Utilities/Trigger.h"
class EnemyTrigger : public Trigger
{
	void OnTrigger(entt::handle handle) override;
};

