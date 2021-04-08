#include "SafeRoomBehaviour.h"

void SafeRoomBehaviour::Update(entt::handle entity)
{
	float dt = Timing::Instance().DeltaTime;
	glm::vec3 pos = entity.get<Transform>().GetLocalPosition();
	if (locked) {
		entity.get<Transform>().SetLocalPosition(glm::vec3(pos.x, 0.0, pos.z));
		locktimer += dt;
		if (locktimer > 8.0f) {
			locktimer = 0.0f;
			MapManager::Instance().set_data(roomi, roomj, true);
			locked = false;
			ready = false;
		}
	}
	else {
		entity.get<Transform>().SetLocalPosition(glm::vec3(pos.x, 1.5, pos.z));
	}
	if (!ready) {
		cdtimer += dt;
		if (cdtimer > 5.0f) {
			cdtimer = 0.0f;
			ready = true;
		}
	}
	
}
