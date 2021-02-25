#include "GameBehaviour.h"
#include "Utilities/Globals.h"

void GameBehaviour::Update(entt::handle entity)
{
	GLFWwindow* window = Application::Instance().Window;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		Application::Instance().ActiveScene = Globals::Instance().scenes[2];
	}
}
