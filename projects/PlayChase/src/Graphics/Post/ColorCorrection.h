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

private:
	float _intensity = 1.0f;
	LUT3D _testCube;
};