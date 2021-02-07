#pragma once
#include <Box2D/box2d.h>
#include <GLM/glm.hpp>
#include <memory>

enum  _type {
	WALL,
	PLAYER,
	BUL,
};
class Collision2D {
public:
	Collision2D(b2World* wrld) : world(wrld) {}
	~Collision2D();

	void CreateDynamicBox(const glm::vec2& position, const glm::vec2& dimensions);
	void CreateStaticBox(const glm::vec2& position, const glm::vec2& dimensions);
	void CreateStaticSensor(const glm::vec2& position, const glm::vec2& dimensions);

	b2Vec2 GetLateralVelocity();

	b2Vec2 GetForwardVelocity();

	void updateFriction();

	void Drive(int controlState);
	void SetAngle(float angle);
	void Shoot();

	void RemoveBody();
	b2Body* getBody() { return body; }
	b2Fixture* getFixture() { return fixture; }

	int colcounter = 0;
private:
	b2World* world = nullptr;
	b2Body* body = nullptr;
	b2Fixture* fixture = nullptr;
	
	float m_maxForwardSpeed = 5;
	float m_maxBackwardSpeed = -3;
	float m_maxDriveForce = 10;
	float m_maxShootSpeed = 20;
	float m_maxShootForce = 30;
};