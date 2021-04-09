#include "ExitBehaviour.h"
#include "Application.h"
#include "Transform.h"
#include "Utilities/Globals.h"
#include "Triggers/ExitTrigger.h"
#include "Utilities/Trigger.h"

void ExitBehaviour::Update(entt::handle entity)
{
	if (Globals::Instance().unlocked) {
		entity.get<RendererComponent>().SetMesh(newtube);
		TriggerBinding::Get<ExitTrigger>(entity)->Enabled = true;
	}
}