#pragma once
#include <entt.hpp>
#include "Collision2D.h"
class Trigger
{
public:
	//Override this in own triggers, make it do something
	virtual void OnTrigger(entt::handle handle);
protected:
	bool _triggered = false;
};

