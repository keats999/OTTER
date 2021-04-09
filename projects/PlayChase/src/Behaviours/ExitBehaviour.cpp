#include "ExitBehaviour.h"
#include "Application.h"
#include "Transform.h"
#include "Utilities/Globals.h"
#include "Triggers/ExitTrigger.h"
#include "Utilities/Trigger.h"

void ExitBehaviour::Update(entt::handle entity)
{
	if (Application::Instance().ActiveScene == Globals::Instance().scenes[1])
	{
		if (Globals::Instance().unlocked) {
			entity.get<RendererComponent>().SetMesh(newtube);
			TriggerBinding::Get<ExitTrigger>(entity)->Enabled = true;
		}
	}
}