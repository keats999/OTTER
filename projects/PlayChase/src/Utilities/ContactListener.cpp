#include "ContactListener.h"
#include "Trigger.h"
#include <entt.hpp>

ContactListener::ContactListener(GameScene::sptr scene)
	: b2ContactListener()
{
	_scene = scene;
}

void ContactListener::BeginContact(b2Contact* contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	bool sensorA = fixtureA->IsSensor();
	bool sensorB = fixtureB->IsSensor();

	if (sensorA ^ sensorB) {
		if (sensorA) {
			TriggerObject(fixtureA);
		}
		if (sensorB) {
			TriggerObject(fixtureB);
		}
	}
}

void ContactListener::EndContact(b2Contact* contact)
{
}

void ContactListener::TriggerObject(b2Fixture* object)
{
	entt::entity entity = object->GetEntity();
	entt::registry& registry = _scene->Registry();
	entt::handle handle = entt::basic_handle(registry, entity);
	if (handle.has<Trigger>()) {
		handle.get<Trigger>().OnTrigger(handle);
	}
}
