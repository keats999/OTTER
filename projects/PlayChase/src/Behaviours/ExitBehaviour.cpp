#include "ExitBehaviour.h"
#include "Application.h"
#include "Transform.h"
#include "Utilities/Globals.h"
#include "Triggers/ExitTrigger.h"
#include "Utilities/Trigger.h"

void ExitBehaviour::Update(entt::handle entity)
{
	if (Globals::Instance().coins == Globals::Instance().coinmax) {
		entity.get<RendererComponent>().SetVisible(true);
		TriggerBinding::Get<ExitTrigger>(entity)->Enabled = true;
	}
}