#include "PhysicsWorld.h"

PhysicsWorld::PhysicsWorld(GameScene::sptr scene)
{
	_scene = scene;
	//Create new Box2D world with 0 gravity
	_world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f));
	_listener = new ContactListener(scene);
	_world->SetContactListener(_listener);
}
void PhysicsWorld::Update(float dt)
{
	//Step the world by the delta time
	_world->Step(dt, 6, 2);

	//Create a view with all entities with both transform components and collision 2D components
	auto view = _scene->Registry().view<Transform, Collision2D>();
	for (auto entity : view) {
		//Gather all data from the physics body
		auto& collider = view.get<Collision2D>(entity);
		auto pos = collider.getBody()->GetPosition();
		auto angle = collider.getBody()->GetAngle();
		//auto contacts = collider.getBody()->GetContactList();
		//for (contacts; contacts; contacts = contacts->next) {}

		//Update transform to match the physics body
		auto& transform = view.get<Transform>(entity);
		glm::vec3 tpos = transform.GetLocalPosition();
		transform.SetLocalPosition(pos.x, tpos.y, pos.y);
		transform.SetLocalRotation(transform.GetLocalRotation().x, -glm::degrees(angle), transform.GetLocalRotation().z);
	}
	CleanupBodies();
}

void PhysicsWorld::CleanupBodies()
{
	entt::registry& reg = _scene->Registry();

	for (int i = 0; i < Collision2D::_bodiesToDelete.size(); i++)
	{
		//Bodies to delete
		reg.get<Collision2D>(Collision2D::_bodiesToDelete[i]).RemoveBody();

		//Destroy the entity
		//reg.destroy(Collision2D::_bodiesToDelete[i]);
		_scene->RemoveEntity(entt::basic_handle(reg, Collision2D::_bodiesToDelete[i]));

		//Clear bodies to delete
		Collision2D::_bodiesToDelete.clear();
	}
}
