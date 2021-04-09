#include "SafeRoomBehaviour.h"
#include "Application.h"
#include "Utilities/Globals.h"

void SafeRoomBehaviour::Update(entt::handle entity)
{
	if (Application::Instance().ActiveScene == Globals::Instance().scenes[1])
	{
		GLFWwindow* window = Application::Instance().Window;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			if (!pressed) {
				if (playercontact) {
					if (locked) {
						pressed = true;
						locktimer = 0.0f;
						Globals::Instance().safe = false;
						MapManager::Instance().set_data(roomi, roomj, true);
						locked = false;
						ready = false;
					}
					else {
						if (ready) {
							pressed = true;
							Globals::Instance().safe = true;
							MapManager::Instance().set_data(roomi, roomj, false);
							locked = true;
						}
					}
				}
			}
		}
		float dt = Timing::Instance().DeltaTime;
		glm::vec3 pos = entity.get<Transform>().GetLocalPosition();
		if (locked) {
			entity.get<Transform>().SetLocalPosition(glm::vec3(pos.x, 0.0, pos.z));
			
			locktimer += dt;
			if (locktimer > 10.0f) {
				locktimer = 0.0f;
				Globals::Instance().safe = false;
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
		if (pressed) {
			presstimer += dt;
			if (presstimer > 0.5f) {
				presstimer = 0.0f;
				pressed = false;
			}
		}
	}
}
