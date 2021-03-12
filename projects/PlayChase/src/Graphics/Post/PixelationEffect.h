#pragma once

#include "Graphics/Post/PostEffect.h"

class PixelationEffect : public PostEffect
{
public:
	void Init(unsigned width, unsigned height) override;
	void ApplyEffect(PostEffect* buffer) override;

	void DrawToScreen() override;

	float GetPercentOfPixels() const;
	void SetPercentOfPixels(float percentOfPixels);
	
private:
	float _percentOfPixels = 1.0f;
};