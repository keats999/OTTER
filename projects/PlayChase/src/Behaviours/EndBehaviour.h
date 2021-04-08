#pragma once
#include "IBehaviour.h"
#include "Application.h"
#include "GLFW/glfw3.h"
class EndBehaviour :
    public IBehaviour
{
public:
    void Update(entt::handle entity) override;
};