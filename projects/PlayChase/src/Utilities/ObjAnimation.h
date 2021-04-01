#pragma once

#include <VertexTypes.h>
#include <iostream>
#include "MeshBuilder.h"


class ObjAnimation {
public:
	ObjAnimation() = default;
	~ObjAnimation() = default;

	void LoadFromFolder(const std::string& folderName, const int numOfFrames, const glm::vec4& inColor = glm::vec4(1.0f));
	void UpdateAnimation(float deltaTime);
	VertexArrayObject::sptr LoadMesh();

private:
	/// <summary>
	/// vertex data of all the frames
	/// first vector	- frame num
	/// second vector	- face num
	/// third vector	- vertex in face num
	/// data			- vertex data
	/// </summary>
	std::vector<std::vector<std::vector<VertexPosNormTexCol>>> vertexInfo;

	/// <summary>
	/// the vertex data in the current frame
	/// first vector	- face num
	/// second vector	- vertex in face num
	/// data			- vertex data
	/// </summary>
	std::vector<std::vector<VertexPosNormTexCol>> currentVertexInfo;

	int currentFrame = 0;
	int nextFrame = 1;
	float totalTime = 0.0f;
};