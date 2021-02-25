#version 410

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

uniform sampler2D s_UiTexture;

uniform vec3  u_CamPos;

out vec4 frag_color;


// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {
	if (texture(s_UiTexture, inUV).a <= 0.1)
		discard;

	frag_color = texture(s_UiTexture, inUV);
}