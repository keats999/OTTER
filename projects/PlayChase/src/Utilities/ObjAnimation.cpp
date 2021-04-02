#include "ObjAnimation.h"

#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>

#include "StringUtils.h"

void ObjAnimation::LoadFromFolder(const std::string& folderName, const int numOfFrames, const glm::vec4& inColor)
{
	for (int frameNum = 1; frameNum <= numOfFrames; frameNum++)
	{
		// Open our file in binary mode
		std::ifstream file;
		file.open("models/" + folderName + "/" + folderName + "_" + std::to_string(frameNum) + ".obj", std::ios::binary);

		// If our file fails to open, we will throw an error
		if (!file) {
			throw std::runtime_error("Failed to open file");
		}

		// Stores attributes
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> textureCoords;

		std::vector<std::vector<VertexPosNormTexCol>> faceVertexArray;

		// We'll use bitmask keys and a map to avoid duplicate vertices
		std::unordered_map<uint64_t, uint32_t> indexMap;

		// Temporaries for loading data
		glm::vec3 temp;
		glm::ivec3 vertexIndices;

		std::string line;
		// Iterate as long as there is content to read
		while (file.peek() != EOF) {
			std::string command;
			file >> command;

			// Load in vertex positions
			if (command == "v") {
				file >> temp.x >> temp.y >> temp.z;
				positions.push_back(temp);
			}
			// Load in vertex normals
			else if (command == "vn") {
				file >> temp.x >> temp.y >> temp.z;
				normals.push_back(temp);
			}
			// Load in UV coordinates
			else if (command == "vt") {
				file >> temp.x >> temp.y;
				textureCoords.push_back(temp);
			}
			// Load in face lines
			else if (command == "f") {
				// Read the entire line, trim it, and stuff it into a string stream
				std::string line;
				std::getline(file, line);
				trim(line);
				std::stringstream stream = std::stringstream(line);

				std::vector<VertexPosNormTexCol> vertexArray;

				//loop through each point info with handling for n-gons
				while(stream.peek() != EOF) {
					// Load in the faces, split up by slashes
					char tempChar;
					vertexIndices = glm::ivec3(0);
					stream >> vertexIndices.x >> tempChar >> vertexIndices.y >> tempChar >> vertexIndices.z;
					vertexIndices -= glm::ivec3(1);

					// Construct a new vertex using the indices for the vertex
					VertexPosNormTexCol vertex;
					vertex.Position = positions[vertexIndices.x];
					vertex.UV = textureCoords[vertexIndices.y];
					vertex.Normal = normals[vertexIndices.z];
					vertex.Color = inColor;

					// Add to the mesh, get index of the added vertex
					vertexArray.push_back(vertex);
				}

				faceVertexArray.push_back(vertexArray);
			}
		}
		vertexInfo.push_back(faceVertexArray);
	}

	if (vertexInfo.size() == 0)
	{
		throw std::runtime_error("unable to gather info from the folder " + folderName + "!\n");
	}

	currentVertexInfo = vertexInfo[0];
}

std::vector<std::vector<VertexPosNormTexCol>> LERP(std::vector<std::vector<VertexPosNormTexCol>> frame1, std::vector<std::vector<VertexPosNormTexCol>> frame2, float t)
{
	std::vector<std::vector<VertexPosNormTexCol>> returnValue;
	for (int i = 0; i < frame1.size(); i++)
	{
		std::vector<VertexPosNormTexCol> faceVertcies;
		for (int j = 0; j < frame1[i].size(); j++)
		{
			VertexPosNormTexCol vertex;

			vertex.Position = frame1[i][j].Position + (frame2[i][j].Position - frame1[i][j].Position) * t;
			vertex.Normal = frame1[i][j].Normal + (frame2[i][j].Normal - frame1[i][j].Normal) * t;
			vertex.UV = frame1[i][j].UV + (frame2[i][j].UV - frame1[i][j].UV) * t;
			vertex.Color = frame1[i][j].Color + (frame2[i][j].Color - frame1[i][j].Color) * t;

			faceVertcies.push_back(vertex);
		}
		returnValue.push_back(faceVertcies);
	}
	return returnValue;
}

void ObjAnimation::UpdateAnimation(float deltaTime)
{
	if (vertexInfo.size() == 0)
	{
		throw std::runtime_error("No animaion to update!\n");
	}
	else if (vertexInfo.size() == 1)
	{
		currentVertexInfo = vertexInfo[0];
	}
	else
	{
		totalTime += deltaTime;
		if (totalTime >= 0.2f)
		{
			totalTime = 0.0f;
			currentFrame = nextFrame;
			nextFrame++;

			if (nextFrame == vertexInfo.size())
			{
				nextFrame = 0;
			}
		}

		currentVertexInfo = LERP(vertexInfo[currentFrame], vertexInfo[nextFrame], totalTime);
	}
}

VertexArrayObject::sptr ObjAnimation::LoadMesh()
{
	if (currentVertexInfo.size() == 0)
	{
		std::cout << "No mesh to load!\n";
		return VertexArrayObject::sptr();
	}

	MeshBuilder<VertexPosNormTexCol> mesh;
	for (int i = 0; i < currentVertexInfo.size(); i++) //for each face
	{
		for (int j = 1; j < currentVertexInfo[i].size() - 1; j++) //vertex num with handling for n-gons
		{
			const uint32_t p1 = mesh.AddVertex(currentVertexInfo[i][0].Position, currentVertexInfo[i][0].Normal, currentVertexInfo[i][0].UV, currentVertexInfo[0][0].Color);
			const uint32_t p2 = mesh.AddVertex(currentVertexInfo[i][j].Position, currentVertexInfo[i][j].Normal, currentVertexInfo[i][j].UV, currentVertexInfo[0][0].Color);
			const uint32_t p3 = mesh.AddVertex(currentVertexInfo[i][j + 1].Position, currentVertexInfo[i][j + 1].Normal, currentVertexInfo[i][j + 1].UV, currentVertexInfo[0][0].Color);
			mesh.AddIndexTri(p1, p2, p3);
		}
	}

	return mesh.Bake();
}