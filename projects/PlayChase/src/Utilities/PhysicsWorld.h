#pragma once
#include <Scene.h>
#include "Utilities/Collision2D.h"
#include <Box2D/box2d.h>
#include <Transform.h>

class PhysicsWorld
{
public:
	PhysicsWorld(const PhysicsWorld& other) = delete;
	PhysicsWorld(PhysicsWorld&& other) = delete;
	PhysicsWorld& operator=(const PhysicsWorld& other) = delete;
	PhysicsWorld& operator=(PhysicsWorld&& other) = delete;
	typedef std::shared_ptr<PhysicsWorld> sptr;
	static inline sptr Create(GameScene::sptr scene) {
		return std::make_shared<PhysicsWorld>(scene);
	}
public:
	PhysicsWorld(GameScene::sptr scene);
	~PhysicsWorld() = default;

	b2World* World() { return _world.get(); }
	void Update(float dt);

private:
	std::unique_ptr<b2World> _world;
	std::shared_ptr<GameScene> _scene;
};