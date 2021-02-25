#include "CoinTrigger.h"

void CoinTrigger::OnTrigger(entt::handle handle)
{
	Collision2D::_bodiesToDelete.push_back(handle.entity());
}
