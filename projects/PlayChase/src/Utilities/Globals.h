#pragma once
#include "Scene.h"
#include <memory>

class Globals {
public:
	static Globals& Instance() {
		static Globals instance;
		return instance;
	}
	int coins = 0;
	int coinmax = 0;
	bool alive = true;
	std::vector<GameScene::sptr> scenes;
};