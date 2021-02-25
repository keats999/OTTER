#include "CoinTrigger.h"
#include "Utilities/Globals.h"

void CoinTrigger::OnTrigger(entt::handle handle)
{
	Globals::Instance().coins++;
	Collision2D::_bodiesToDelete.push_back(handle.entity());
}
