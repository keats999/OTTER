#include "EnemyTrigger.h"
#include "Utilities/Globals.h"
#include "Application.h"
#include "GLFW/glfw3.h"
#include "Utilities/Collision2D.h"

void EnemyTrigger::OnTrigger(entt::handle handle)
{
	Globals::Instance().alive = false;
	GLFWwindow* window = Application::Instance().Window;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	for (int i = 0; i < Globals::Instance().coinArray.size(); i++)
	{
		Collision2D::_bodiesToDelete.push_back(Globals::Instance().coinArray[i]);
	}
	Globals::Instance().coinArray.clear();
	Application::Instance().ActiveScene = Globals::Instance().scenes[4];
}