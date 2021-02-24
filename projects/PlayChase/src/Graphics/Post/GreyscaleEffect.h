#pragma once

#include "Graphics/Post/PostEffect.h"

class GreyscaleEffect : public PostEffect
{
public:
	void Init(unsigned width, unsigned height) override;
	void ApplyEffect(PostEffect* buffer) override;

	void DrawToScreen() override;

	float GetIntensity() const;
	void SetIntensity(float intensity);
	
private:
	float _intensity = 0.0f;
};