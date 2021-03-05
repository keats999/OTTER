#pragma once
#include "IBehaviour.h"
#include "Camera.h"

class UIBehaviour : public IBehaviour
{
public:
	void Update(entt::handle entity) override;
	void SetCamera(entt::handle parent) { _parent = parent.entity(); }

protected:
	entt::entity _parent{ entt::null };
};

