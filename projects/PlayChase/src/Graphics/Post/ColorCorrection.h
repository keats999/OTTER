#pragma once

#include "Graphics/Post/PostEffect.h"
#include "Graphics/LUT.h"

class ColorCorrection : public PostEffect
{
public:
	void Init(unsigned width, unsigned height) override;
	void ApplyEffect(PostEffect* buffer) override;

	float GetIntensity() const;
	void SetIntensity(float intensity);

	LUT3D _testCube;
	
	//void reloadCube(unsigned width, unsigned height);
	std::string filename;
private:
	float _intensity = 1.0f;
};