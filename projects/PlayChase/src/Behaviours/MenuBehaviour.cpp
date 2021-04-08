#include "MenuBehaviour.h"
#include "Utilities/Globals.h"
#include "glad/glad.h"

void MenuBehaviour::Update(entt::handle entity)
{
	GLFWwindow* window = Application::Instance().Window;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		Application::Instance().ActiveScene = Globals::Instance().scenes[1];
	}
}