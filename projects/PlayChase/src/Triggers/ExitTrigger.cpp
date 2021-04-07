#include "ExitTrigger.h"
void ExitTrigger::OnTrigger(entt::handle handle)
{
	Application::Instance().ActiveScene = Globals::Instance().scenes[3];
}