#include "UIBehaviour.h"
#include "Transform.h"
#include "Utilities/BackendHandler.h"

void UIBehaviour::Update(entt::handle entity)
{
	entt::registry& reg = entity.registry();
	auto cam = reg.get<Transform>(_parent);
	entity.get<Transform>().SetLocalPosition(cam.GetLocalPosition() + (glm::vec3(-0.11f, 0.0f, -0.11f) * glm::vec3(sin(glm::radians((int(cam.GetLocalRotation().x) != 180) ? cam.GetLocalRotation().y : -cam.GetLocalRotation().y + cam.GetLocalRotation().x)), 0.0f, cos(glm::radians((int(cam.GetLocalRotation().x) != 180) ? cam.GetLocalRotation().y : -cam.GetLocalRotation().y + cam.GetLocalRotation().x)))));
	entity.get<Transform>().SetLocalRotation(90.0f, (int(cam.GetLocalRotation().x) != 180) ? cam.GetLocalRotation().y : -cam.GetLocalRotation().y + cam.GetLocalRotation().x, 0.0f);
	entity.get<Transform>().SetLocalScale(0.11f * BackendHandler::aspectRatio, 0.11f, 0.11f);
}
