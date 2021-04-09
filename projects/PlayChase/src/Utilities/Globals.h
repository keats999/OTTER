#pragma once
#include "Scene.h"
#include <memory>
#include <Camera.h>

class Globals {
public:
	static Globals& Instance() {
		static Globals instance;
		return instance;
	}
	int coins = 0;
	int coinmax = 0;
	bool alive = true;
	bool unlocked = false;
	bool safe = false;
	std::vector<GameObject> coinArray;
	std::vector<GameScene::sptr> scenes;
};