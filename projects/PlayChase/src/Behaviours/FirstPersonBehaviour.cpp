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
	
	/*
	* 	Camera& camera = entity.get<Camera>();
	// mouse movement
	tagPOINT mousePos;
	GetCursorPos(&mousePos);
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	int width = desktop.right;
	int height = desktop.bottom;
	
	if (_trapped) {
		_mouseX += mousePos.x - (width / 2);
		if (_mouseX > 0)
			while (_mouseX >= 360 * 20.0f)
				_mouseX -= 360.0f * 20.0f;
		else
			while (_mouseX <= -360 * 20.0f)
				_mouseX += 360.0f * 20.0f;
		//camera.LookAt(camera.GetPosition() + glm::vec3(glm::cos(glm::radians((float)_mouseX / 20.0f)), 0.0f, glm::sin(glm::radians((float)_mouseX / 20.0f))));

		reg.get<Collision2D>(_parent).SetAngle(glm::radians(((float)_mouseX / 20.0f) - 90.0f));
		SetCursorPos(width / 2, height / 2);

	}*/
	/*if (_trapped) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		_isPressed = true;
		_prevMouseX = mx;
		_prevMouseY = my;
	}*/

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	_rotationX += static_cast<float>(mx - _prevMouseX) * 0.5f;
	_rotationY += static_cast<float>(my - _prevMouseY) * 0.3f;
	glm::quat rotX = glm::angleAxis(glm::radians(-_rotationX), glm::vec3(0, 1, 0));
	glm::quat rotY = glm::angleAxis(glm::radians(-_rotationY), glm::vec3(1, 0, 0));
	transform.SetLocalRotation(rotX * rotY);
	reg.get<Collision2D>(_parent).SetAngle(glm::radians(_rotationX));
	_prevMouseX = mx;
	_prevMouseY = my;

}
