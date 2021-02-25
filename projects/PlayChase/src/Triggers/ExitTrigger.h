#pragma once
#include "Utilities/Trigger.h"
#include "Application.h"
#include "Utilities/Globals.h"

class ExitTrigger : public Trigger
{
	void OnTrigger(entt::handle handle) override;
};
