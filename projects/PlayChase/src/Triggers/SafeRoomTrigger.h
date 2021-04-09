#pragma once
#include "Utilities/Trigger.h"
#include <GLM/glm.hpp>

class SafeRoomTrigger : public Trigger
{
public:
	void OnTrigger(entt::handle handle) override;
	void EndTrigger(entt::handle handle) override;
};

