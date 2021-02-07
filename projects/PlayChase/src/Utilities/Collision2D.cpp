#include "Collision2D.h"


Collision2D::~Collision2D()
{
	//world->DestroyBody(body);
}


void Collision2D::CreateDynamicBox(const glm::vec2& position, const glm::vec2& dimensions)
{
	//Dynamic box construction (for moving solid bodies)
	//Create body definition
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(position.x, position.y);
	body = world->CreateBody(&bodyDef);

	//Create collision shape
	b2PolygonShape boxShape;
	boxShape.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0);

	//Create fixture definition
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixtureDef.filter.groupIndex = -1;
	fixture = body->CreateFixture(&fixtureDef);
}
void Collision2D::CreateStaticBox(const glm::vec2& position, const glm::vec2& dimensions)
{
	//Static box construction (for unmoving solid bodies)
	//Create body definition
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(position.x, position.y);
	body = world->CreateBody(&bodyDef);

	//Create collision shape
	b2PolygonShape boxShape;
	boxShape.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0);

	//Create fixture definition
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixtureDef.filter.groupIndex = 0;
	fixture = body->CreateFixture(&fixtureDef);
}
void Collision2D::CreateStaticSensor(const glm::vec2& position, const glm::vec2& dimensions)
{
	//Static sensor box construction (for non-solid collision areas)
	//Create body definition
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(position.x, position.y);
	body = world->CreateBody(&bodyDef);

	//Create collision shape
	b2PolygonShape boxShape;
	boxShape.SetAsBox(dimensions.x / 2.0f, dimensions.y / 2.0);

	//Create fixture definition
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;
	fixtureDef.filter.groupIndex = 0;
	fixtureDef.isSensor = true;
	fixture = body->CreateFixture(&fixtureDef);
	
}
b2Vec2 Collision2D::GetLateralVelocity() {
	b2Vec2 currentRightNormal = body->GetWorldVector(b2Vec2(1, 0));
	return b2Dot(currentRightNormal, body->GetLinearVelocity()) * currentRightNormal;
}
b2Vec2 Collision2D::GetForwardVelocity() {
	b2Vec2 currentForwardNormal = body->GetWorldVector(b2Vec2(0, 1));
	return b2Dot(currentForwardNormal, body->GetLinearVelocity()) * currentForwardNormal;
}

void Collision2D::updateFriction() {
	//lateral linear velocity
	b2Vec2 impulse = body->GetMass() * -GetLateralVelocity();
	body->ApplyLinearImpulse(impulse, body->GetWorldCenter(), true);

	//angular velocity
	body->ApplyAngularImpulse(0.2f * body->GetInertia() * -body->GetAngularVelocity(), true);

	//forward linear velocity
	b2Vec2 currentForwardNormal = GetForwardVelocity();
	float currentForwardSpeed = currentForwardNormal.Normalize();
	float dragForceMagnitude = -2 * currentForwardSpeed;
	body->ApplyForce(dragForceMagnitude * currentForwardNormal, body->GetWorldCenter(), true);
}
void Collision2D::Drive(int controlState) {
	//find desired speed
	float desiredSpeed = 0;
	float desiredTorque = 0;

	switch (controlState) {
	case 3:   desiredSpeed = m_maxForwardSpeed;  break;
	case 4: desiredSpeed = m_maxBackwardSpeed; break;
	case 1:  desiredTorque = -10.0;  break;
	case 2: desiredTorque = 10.0; break;
	default: return;//do nothing
	}

	//find current speed in forward direction
	b2Vec2 currentForwardNormal = body->GetWorldVector(b2Vec2(0, 1));
	float currentSpeed = b2Dot(GetForwardVelocity(), currentForwardNormal);

	//apply necessary force
	float force = 0;
	if (desiredSpeed > currentSpeed)
		force = m_maxDriveForce;
	else if (desiredSpeed < currentSpeed)
		force = -m_maxDriveForce;
	else
		return;
	body->ApplyForce(force * currentForwardNormal, body->GetWorldCenter(), true);
}
void Collision2D::SetAngle(float angle)
{
	body->SetAngle(angle);
}
void Collision2D::Shoot() {

	b2Vec2 currentForwardNormal = body->GetWorldVector(b2Vec2(0, 1));
	float currentSpeed = b2Dot(GetForwardVelocity(), currentForwardNormal);

	body->ApplyForce(m_maxShootForce * currentForwardNormal, body->GetWorldCenter(), true);

}
void Collision2D::RemoveBody() {
	world->DestroyBody(body);
	body = nullptr;
}
