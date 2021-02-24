#include "Trigger.h"
#include <iostream>
void Trigger::OnTrigger(entt::handle handle)
{
	_triggered = true;
}
