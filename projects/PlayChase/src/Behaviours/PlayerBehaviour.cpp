#include "PlayerBehaviour.h"

#pragma once

void PlayerBehaviour::Update(entt::handle entity)
{
	GLFWwindow* window = Application::Instance().Window;
	int controlState = 0;

	AudioEngine& engine = AudioEngine::Instance();
	AudioEvent& thumping = engine.GetEvent("Player Thumping");
	bool moving = false;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		controlState = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		controlState = 2;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		controlState = 4;
		moving = true;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		controlState = 3;
		moving = true;
	}

	thumping.SetParameter("Moving", (int)moving);

	Collision2D& collider = entity.get<Collision2D>();

	float desiredSpeed = 0;
	float desiredTorque = 0;

	switch (controlState) {
	case 3:   desiredSpeed = _maxForwardSpeed;  break;
	case 4: desiredSpeed = _maxBackwardSpeed; break;
	case 1:  desiredTorque = -10.0;  break;
	case 2: desiredTorque = 10.0; break;
	default: return;//do nothing
	}

	//find current speed in forward direction
	b2Vec2 currentForwardNormal = collider.getBody()->GetWorldVector(b2Vec2(0, 1));
	float currentSpeed = b2Dot(collider.GetForwardVelocity(), currentForwardNormal);

	//apply necessary force
	float force = 0;
	if (desiredSpeed > currentSpeed)
		force = _maxForce;
	else if (desiredSpeed < currentSpeed)
		force = -_maxForce;
	else
		return;
	collider.getBody()->ApplyForce(force * currentForwardNormal, collider.getBody()->GetWorldCenter(), true);
}
