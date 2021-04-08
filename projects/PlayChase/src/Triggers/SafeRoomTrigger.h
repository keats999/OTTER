#pragma once
#include "Utilities/Trigger.h"
#include <GLM/glm.hpp>
#include "Utilities/MapManager.h"

class SafeRoomTrigger : public Trigger
{
public:
	void OnTrigger(entt::handle handle) override;
	void SetRoom(int i, int j) { roomi = i; roomj = j; }
private:
	int roomi = -1;
	int roomj = -1;
};

