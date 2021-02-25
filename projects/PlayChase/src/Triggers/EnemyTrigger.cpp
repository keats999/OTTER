#include "EnemyTrigger.h"
#include "Utilities/Globals.h"

void EnemyTrigger::OnTrigger(entt::handle handle)
{
	Globals::Instance().alive = false;
}