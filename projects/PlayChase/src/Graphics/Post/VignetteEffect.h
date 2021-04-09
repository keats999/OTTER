#pragma once

#include "Graphics/Post/PostEffect.h"

class VignetteEffect : public PostEffect
{
public:
	void Init(unsigned width, unsigned height) override;
	void ApplyEffect(PostEffect* buffer) override;

	void DrawToScreen() override;

	float GetIntensity() const;
	void SetIntensity(float intensity);
	
private:
	float _intensity = 1.0f;
	float _innerRadius = 0.5f;
	float _outerRadius = 0.9f;
};