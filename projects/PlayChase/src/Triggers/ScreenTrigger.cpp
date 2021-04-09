#include "ScreenTrigger.h"
#include "IBehaviour.h"
#include "Behaviours/ScreenBehaviour.h"


void ScreenTrigger::OnTrigger(entt::handle handle)
{
	BehaviourBinding::Get<ScreenBehaviour>(handle)->playercontact = true;
}

void ScreenTrigger::EndTrigger(entt::handle handle)
{
	BehaviourBinding::Get<ScreenBehaviour>(handle)->playercontact = false;
}
