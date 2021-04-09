#include "CoinTrigger.h"
#include "Transform.h"
#include "Utilities/Globals.h"
#include "Utilities/AudioEngine.h"

void CoinTrigger::OnTrigger(entt::handle handle)
{
	AudioEngine& engine = AudioEngine::Instance();
	AudioEvent& coinCollect = engine.GetEvent("Coin Collect");

	coinCollect.SetPosition(handle.get<Transform>().GetLocalPosition());
	coinCollect.Play();
	Globals::Instance().coins++;
	Collision2D::_bodiesToDelete.push_back(handle.entity());
	for (int i = 0; i < Globals::Instance().coinArray.size(); i++)
	{
		if (Globals::Instance().coinArray[i].entity() == handle.entity())
		{
			Globals::Instance().coinArray.erase(Globals::Instance().coinArray.begin() + i);
			break;
		}
	}
}
