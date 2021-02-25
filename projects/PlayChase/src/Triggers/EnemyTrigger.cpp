#include "EnemyTrigger.h"
#include "Utilities/Globals.h"
#include "Application.h"
#include "GLFW/glfw3.h"

void EnemyTrigger::OnTrigger(entt::handle handle)
{
	Globals::Instance().alive = false;
	GLFWwindow* window = Application::Instance().Window;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	Application::Instance().ActiveScene = Globals::Instance().scenes[4];
}