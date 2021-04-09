#include "ScreenBehaviour.h"
#include "Utilities/Globals.h"


void ScreenBehaviour::Update(entt::handle entity)
{
	if (Application::Instance().ActiveScene == Globals::Instance().scenes[1])
	{
		GLFWwindow* window = Application::Instance().Window;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			if (!pressed) {
				if (playercontact) {
					int count = Globals::Instance().coinmax - Globals::Instance().coins;
					screenMat->Set("s_Diffuse", screens[count]);
				}
				if (Globals::Instance().coins == Globals::Instance().coinmax) {
					Globals::Instance().unlocked = true;
				}
			}
		}
		float dt = Timing::Instance().DeltaTime;

		if (pressed) {
			presstimer += dt;
			if (presstimer > 0.5f) {
				presstimer = 0.0f;
				pressed = false;
			}
		}
	}
}

void ScreenBehaviour::SetMaterial(ShaderMaterial::sptr mat)
{
	screenMat = mat;
	screens.push_back(Texture2D::LoadFromFile("images/screens/screen0.png"));
	screens.push_back(Texture2D::LoadFromFile("images/screens/screen1.png"));
	screens.push_back(Texture2D::LoadFromFile("images/screens/screen2.png"));
	screens.push_back(Texture2D::LoadFromFile("images/screens/screen3.png"));
	screens.push_back(Texture2D::LoadFromFile("images/screens/screen4.png"));
	screens.push_back(Texture2D::LoadFromFile("images/screens/screen5.png"));
	screens.push_back(Texture2D::LoadFromFile("images/screens/screen6.png"));
	screens.push_back(Texture2D::LoadFromFile("images/screens/screen7.png"));
	screens.push_back(Texture2D::LoadFromFile("images/screens/screen8.png"));
	screens.push_back(Texture2D::LoadFromFile("images/screens/screen9.png"));
	int count = Globals::Instance().coinmax - Globals::Instance().coins;
	screenMat->Set("s_Diffuse", screens[count]);
}
