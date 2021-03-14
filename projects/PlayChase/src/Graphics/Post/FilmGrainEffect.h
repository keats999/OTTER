#pragma once

#include "Graphics/Post/PostEffect.h"

class FilmGrainEffect : public PostEffect
{
public:
	void Init(unsigned width, unsigned height) override;
	void ApplyEffect(PostEffect* buffer) override;

	void DrawToScreen() override;

	float GetIntensity() const;
	void SetIntensity(float intensity);
	void SetTime(float timeNum);
	
private:
	float _intensity = 0.5f;
	float _time = 0.0f;
};