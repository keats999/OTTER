#include "SafeRoomTrigger.h"
#include "IBehaviour.h"
#include "Behaviours/SafeRoomBehaviour.h"
#include "Utilities/MapManager.h"

void SafeRoomTrigger::OnTrigger(entt::handle handle)
{

	BehaviourBinding::Get<SafeRoomBehaviour>(handle)->playercontact = true;
}

void SafeRoomTrigger::EndTrigger(entt::handle handle)
{
	BehaviourBinding::Get<SafeRoomBehaviour>(handle)->playercontact = false;
}
