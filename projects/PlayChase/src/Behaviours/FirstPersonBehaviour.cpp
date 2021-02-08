#include "FirstPersonBehaviour.h"

#include "Application.h"
#include "Timing.h"
#include "Transform.h"

void FirstPersonBehaviour::Update(entt::handle entity)
{
	float dt = Timing::Instance().DeltaTime;
	GLFWwindow* window = Application::Instance().Window;
	double mx, my;
	glfwGetCursorPos(window, &mx, &my);
	
	entt::registry& reg = entity.registry();
	auto playpos = reg.get<Transform>(_parent).GetLocalPosition();
	
	Transform& transform = entity.get<Transform>();
	transform.SetLocalPosition(playpos);
	if (_trapped) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		_rotationX += static_cast<float>(mx - _prevMouseX) * 0.5f;
		_rotationY += static_cast<float>(my - _prevMouseY) * 0.3f;
		glm::quat rotX = glm::angleAxis(glm::radians(-_rotationX), glm::vec3(0, 1, 0));
		glm::quat rotY = glm::angleAxis(glm::radians(-_rotationY), glm::vec3(1, 0, 0));
		transform.SetLocalRotation(rotX);
		reg.get<Collision2D>(_parent).SetAngle(glm::radians(_rotationX));
		_prevMouseX = mx;
		_prevMouseY = my;
	}
	else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

}
