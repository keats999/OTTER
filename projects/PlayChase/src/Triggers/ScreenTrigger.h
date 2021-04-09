#pragma once
#include "Utilities/Trigger.h"
#include <GLM/glm.hpp>

class ScreenTrigger : public Trigger
{
public:
	void OnTrigger(entt::handle handle) override;
	void EndTrigger(entt::handle handle) override;
};

