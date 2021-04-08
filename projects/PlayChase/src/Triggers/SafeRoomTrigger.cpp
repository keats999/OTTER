#include "SafeRoomTrigger.h"
#include "IBehaviour.h"
#include "Behaviours/SafeRoomBehaviour.h"
#include "Utilities/MapManager.h"

void SafeRoomTrigger::OnTrigger(entt::handle handle)
{

	if (!BehaviourBinding::Get<SafeRoomBehaviour>(handle)->locked && BehaviourBinding::Get<SafeRoomBehaviour>(handle)->ready) {
		MapManager::Instance().set_data(roomi, roomj, false);
		BehaviourBinding::Get<SafeRoomBehaviour>(handle)->locked = true;
	}
}
