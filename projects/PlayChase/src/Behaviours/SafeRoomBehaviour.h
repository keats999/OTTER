#pragma once
#include "IBehaviour.h"
#include <GLM/glm.hpp>
#include "Timing.h"
#include "Utilities/MapManager.h"
#include "Transform.h"
#include "GLFW/glfw3.h"
#include "Application.h"

class SafeRoomBehaviour : public IBehaviour
{
public:
	void Update(entt::handle entity) override;
	void SetRoom(int i, int j) { roomi = i; roomj = j; }
	bool playercontact = false;
private:
	bool locked = false;
	bool ready = true;
	bool pressed = false;
	int roomi = -1;
	int roomj = -1;
	float locktimer = 0.0f;
	float cdtimer = 0.0f;
	float presstimer = 0.0f;
};

