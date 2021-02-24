#pragma once
#include <Box2D/box2d.h>
#include "Scene.h"
class ContactListener : public b2ContactListener
{
public:
	ContactListener(GameScene::sptr scene);
	void BeginContact(b2Contact* contact) override;
	void EndContact(b2Contact* contact) override;
private:
	void TriggerObject(b2Fixture* object);
	std::shared_ptr<GameScene> _scene;
};

