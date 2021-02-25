#pragma once
#include "Utilities/Trigger.h"
#include "Utilities/Collision2D.h"
#include <Application.h>
class CoinTrigger : public Trigger
{
public:
	void OnTrigger(entt::handle handle) override;
};

