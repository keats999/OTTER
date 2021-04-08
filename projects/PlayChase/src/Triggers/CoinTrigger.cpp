#include "CoinTrigger.h"
#include "Utilities/Globals.h"

void CoinTrigger::OnTrigger(entt::handle handle)
{
	Globals::Instance().coins++;
	Collision2D::_bodiesToDelete.push_back(handle.entity());
	for (int i = 0; i < Globals::Instance().coinArray.size(); i++)
	{
		if (Globals::Instance().coinArray[i].entity() == handle)
		{
			Globals::Instance().coinArray.erase(Globals::Instance().coinArray.begin() + i);
			break;
		}
	}
}
