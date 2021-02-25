#include "ExitTrigger.h"
void ExitTrigger::OnTrigger(entt::handle handle)
{
	if (Globals::Instance().coins == Globals::Instance().coinmax) {
		Application::Instance().ActiveScene = Globals::Instance().scenes[3];
	}
}