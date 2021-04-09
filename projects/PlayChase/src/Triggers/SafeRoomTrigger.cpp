#include "SafeRoomTrigger.h"
#include "IBehaviour.h"
#include "Behaviours/SafeRoomBehaviour.h"
#include "Utilities/MapManager.h"
#include "Utilities/AudioEngine.h"

void SafeRoomTrigger::OnTrigger(entt::handle handle)
{
	AudioEngine& engine = AudioEngine::Instance();
	AudioEvent& door = engine.GetEvent("Door");
	door.SetPosition(handle.get<Transform>().GetLocalPosition());
	door.SetParameter("Opening", 0);
	door.Play();
	BehaviourBinding::Get<SafeRoomBehaviour>(handle)->playercontact = true;
}

void SafeRoomTrigger::EndTrigger(entt::handle handle)
{
	AudioEngine& engine = AudioEngine::Instance();
	AudioEvent& door = engine.GetEvent("Door");
	door.SetPosition(handle.get<Transform>().GetLocalPosition());
	door.SetParameter("Opening", 1);
	door.Play();
	BehaviourBinding::Get<SafeRoomBehaviour>(handle)->playercontact = false;
}
