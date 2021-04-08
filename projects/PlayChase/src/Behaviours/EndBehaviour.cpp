#include "EndBehaviour.h"
#include "Utilities/Globals.h"

void EndBehaviour::Update(entt::handle entity)
{
	GLFWwindow* window = Application::Instance().Window;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		Application::Instance().ActiveScene = Globals::Instance().scenes[1];
	}
}