#include "MapManager.h"

// Borrowed from https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
#pragma region String Trimming

// trim from start (in place)
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}

#pragma endregion 

void MapManager::LoadFromFile(const std::string& filename)
{
	// Open our file in binary mode
	std::ifstream file;
	file.open(filename, std::ios::binary);

	// If our file fails to open, we will throw an error
	if (!file) {
		throw std::runtime_error("Failed to open file");
	}

	std::string line;
	std::istringstream ss;
	int lcount = 1;
	

	while (std::getline(file, line)) {
		trim(line);
		std::vector<int> linevec;
		
		if (line.substr(0, 1) == "s")
		{
			ss = std::istringstream(line.substr(2));
			ss >> row >> col;
			row += 2;
			col += 2;
			mapdata.resize(row, std::vector<int>(col, 0));
			init = true;
		}
		else {
			if(init){
				ss = std::istringstream(line);
				mapdata.push_back(linevec);
				int ccount = 1;
				while (!ss.eof()) {
					int temp;
					ss >> temp;
					mapdata[lcount][ccount] = temp;
					ccount++;
				}
				lcount++;
			}
		}
	}
}

glm::vec2 MapManager::GetTube(glm::vec2 pos)
{
	// Room = 0 | Straight = 1 | Elbow = 2 | Tee = 3 | Quad = 4
	int piece = 0;
	float angle = 0.0f;
	int nghbrs[4];
	nghbrs[0] = mapdata[pos.x + 1][pos.y];
	nghbrs[1] = mapdata[pos.x][pos.y - 1];
	nghbrs[2] = mapdata[pos.x - 1][pos.y];
	nghbrs[3] = mapdata[pos.x][pos.y + 1];
	int nsum = 0;
	for (int i = 0; i < 4; i++) {
		nsum += nghbrs[i];
	}
	if (nsum > 2) {
		piece = nsum;
	}
	else if (nsum == 2) {
		if (nghbrs[0] == nghbrs[2] && nghbrs[1] == nghbrs[3]) {
			piece = 1;
		}
		else {
			piece = 2;
		}
	}
	switch (piece) {
	case 0:
		if (nghbrs[2] == 1) {
			angle = 90.0f;
		}
		else if (nghbrs[1] == 1) {
			angle = 180.0f;
		}
		else if (nghbrs[0] == 1) {
			angle = 270.0f;
		}
		break;
	case 1:
		if (nghbrs[0] == 1) {
			angle = 90.0f;
		}
		break;
	case 2:
		if (nghbrs[0] == 1 && nghbrs[1] == 1) {
			angle = 180.0f;
		}
		else if(nghbrs[1] == 1 && nghbrs[2] == 1){
			angle = 90.0f;
		}
		else if (nghbrs[3] == 1 && nghbrs[0] == 1) {
			angle = 270.0f;
		}
		break;
	case 3:
		if (nghbrs[1] == nghbrs[3]) {
			if (nghbrs[0] == 1) {
				angle = 270.0f;
			}
			else {
				angle = 90.0f;
			}
		}
		else {
			if (nghbrs[1] == 1) {
				angle = 180.0f;
			}
		}
		break;
	default:
		angle = 0.0f;
		break;
	}
	return glm::vec2(piece, angle);
}
glm::vec2 MapManager::TrnsToPos(glm::vec3 pos) {
	glm::vec2 p;
	p.x = int(round(pos.x) / unitsize);
	p.y = int(round(pos.z) / unitsize);
	return p;
}
glm::vec3 MapManager::PosToTrns(glm::vec2 pos) {
	glm::vec3 p;
	p.x = int(pos.x * unitsize);
	p.y = 0;
	p.z = int(pos.y * unitsize);
	return p;
}
//Borrowed from https://dev.to/jansonsa/a-star-a-path-finding-c-4a4h
bool MapManager::ValidNode(int x, int y) { 
	if (x < 0 || y < 0 || x >= col || y >= row) {
		return false;
	}
	else {
		if (mapdata[x][y] == 1) {
			return true;
		}
	}
	return false;
}
bool  MapManager::isDestination(int x, int y, Node dest) {
	if (x == dest.pos.x && y == dest.pos.y) {
		return true;
	}
	return false;
}
double  MapManager::calculateH(int x, int y, Node dest) {
	double H = (sqrt((x - dest.pos.x) * (x - dest.pos.x)
		+ (y - dest.pos.y) * (y - dest.pos.y)));
	return H;
}
std::vector<glm::vec3> MapManager::NodeListToVecList(std::vector<Node> nodes) {
	std::vector<glm::vec3> positions;
	std::vector<Node>::iterator itNode;
	for (std::vector<Node>::iterator it = nodes.begin();
		it != nodes.end(); it = next(it)) {
		Node n = *it;
		positions.push_back(PosToTrns(n.pos));
	}
	return positions;
	
}
std::vector<glm::vec3> MapManager::aStar(glm::vec3 start, glm::vec3 end) {
	Node player; 
	Node dest;
	player.pos = TrnsToPos(start);
	dest.pos = TrnsToPos(end);
	std::vector<glm::vec3> empty;
	if (ValidNode(dest.pos.x, dest.pos.y) == false) {
		return empty;

	}
	if (isDestination(player.pos.x, player.pos.y, dest)) {
		empty.push_back(start);
		empty.push_back(end);
		return empty;
	}
	std::vector<std::vector<bool>> closedList;
	closedList.resize(row, std::vector<bool>(col));

	std::vector<std::vector<Node>> allMap;
	allMap.resize(row, std::vector<Node>(col));
	for (int x = 0; x < col; x++) {
		for (int y = 0; y < row; y++) {
			allMap[x][y].fCost = FLT_MAX;
			allMap[x][y].gCost = FLT_MAX;
			allMap[x][y].hCost = FLT_MAX;
			allMap[x][y].parent = glm::vec2(-1, -1);
			allMap[x][y].pos = glm::vec2(x, y);
			
			closedList[x][y] = false;
		}
	}

	int x = player.pos.x;
	int y = player.pos.y;
	allMap[x][y].fCost = 0.0;
	allMap[x][y].gCost = 0.0;
	allMap[x][y].hCost = 0.0;
	allMap[x][y].parent = player.pos;

	std::vector<Node> openList;
	openList.emplace_back(allMap[x][y]);
	bool destinationFound = false;

	while (!openList.empty() && openList.size() < col * row) {
		Node node;
		do {
			float temp = FLT_MAX;
			std::vector<Node>::iterator itNode;
			for (std::vector<Node>::iterator it = openList.begin();
				it != openList.end(); it = next(it)) {
				Node n = *it;
				if (n.fCost < temp) {
					temp = n.fCost;
					itNode = it;
				}
			}
			node = *itNode;
			openList.erase(itNode);
		} while (ValidNode(node.pos.x, node.pos.y) == false && !openList.empty());

		x = node.pos.x;
		y = node.pos.y;
		closedList[x][y] = true;

		//For each neighbour starting from North-West to South-East
		for (int newX = -1; newX <= 1; newX++) {
			for (int newY = -1; newY <= 1; newY++) {
				if (abs(newX) + abs(newY) < 2) {
					double gNew, hNew, fNew;
					if (ValidNode(x + newX, y + newY)) {
						if (isDestination(x + newX, y + newY, dest))
						{
							//Destination found - make path
							allMap[x + newX][y + newY].parent.x = x;
							allMap[x + newX][y + newY].parent.y = y;
							destinationFound = true;
							return NodeListToVecList(makePath(allMap, dest));
						}
						else if (closedList[x + newX][y + newY] == false)
						{
							gNew = node.gCost + 1.0;
							hNew = calculateH(x + newX, y + newY, dest);
							fNew = gNew + hNew;
							// Check if this path is better than the one already present
							if (allMap[x + newX][y + newY].fCost == FLT_MAX ||
								allMap[x + newX][y + newY].fCost > fNew)
							{
								// Update the details of this neighbour node
								allMap[x + newX][y + newY].fCost = fNew;
								allMap[x + newX][y + newY].gCost = gNew;
								allMap[x + newX][y + newY].hCost = hNew;
								allMap[x + newX][y + newY].parent.x = x;
								allMap[x + newX][y + newY].parent.y = y;
								openList.emplace_back(allMap[x + newX][y + newY]);
							}
						}
					}
				}
			}
		}
	}
	if (destinationFound == false) {
		return empty;
	}
}
std::vector<Node> MapManager::makePath(std::vector<std::vector<Node>> map, Node dest) {
	try {
		int x = dest.pos.x;
		int y = dest.pos.y;
		std::stack<Node> path;
		std::vector<Node> usablePath;

		while (!(map[x][y].parent.x == x && map[x][y].parent.y == y)
			&& map[x][y].pos.x != -1 && map[x][y].pos.y != -1)
		{
			path.push(map[x][y]);
			int tempX = map[x][y].parent.x;
			int tempY = map[x][y].parent.y;
			x = tempX;
			y = tempY;

		}
		path.push(map[x][y]);

		while (!path.empty()) {
			Node top = path.top();
			path.pop();
			usablePath.emplace_back(top);
		}
		return usablePath;
	}
	catch (const std::exception& e) {
	}
}