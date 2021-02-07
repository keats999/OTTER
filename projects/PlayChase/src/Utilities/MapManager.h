#pragma once
#include <sstream>
#include <fstream>
#include <iostream>
#include <exception>
#include <algorithm>
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include <GLM/glm.hpp>
struct Node
{
	glm::vec2 pos;
	glm::vec2 parent;
	float gCost;
	float hCost;
	float fCost;
};

inline bool operator < (const Node& lhs, const Node& rhs)
{//We need to overload "<" to put our struct into a set
	return lhs.fCost < rhs.fCost;
}

class MapManager {
public:
	MapManager(const MapManager& other) = delete;
	MapManager(MapManager&& other) = delete;
	MapManager& operator=(const MapManager& other) = delete;
	MapManager& operator=(MapManager&& other) = delete;

	typedef std::shared_ptr<MapManager> sptr;
	static inline sptr Create() {
		return std::make_shared<MapManager>();
	}
public:
	MapManager() = default;
	~MapManager() = default;
	
	void LoadFromFile(const std::string& filename);
	
	std::vector<std::vector<int>>& GetMap() { return mapdata; }
	
	int GetRows() { return row; }
	int GetColumns() { return col; }
	int GetUnitS() { return unitsize; }
	bool ValidNode(int x, int y);
	bool isDestination(int x, int y, Node dest);
	double calculateH(int x, int y, Node dest);
	std::vector<glm::vec3> aStar(glm::vec3 start, glm::vec3 end);
	std::vector<Node> makePath(std::vector<std::vector<Node>> map, Node dest);
	glm::vec2 GetTube(glm::vec2 pos);
	glm::vec2 TrnsToPos(glm::vec3 pos);
	glm::vec3 PosToTrns(glm::vec2 pos);
	std::vector<glm::vec3> NodeListToVecList(std::vector<Node> nodes);
	
private:
	int row = 0, col = 0;
	int unitsize = 2;
	bool init = false;
	std::vector<std::vector<int>> mapdata;
};