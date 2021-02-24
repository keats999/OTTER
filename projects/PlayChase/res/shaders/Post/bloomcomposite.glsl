#version 420
layout(binding = 0) uniform sampler2D u_Scene;
layout(binding = 1) uniform sampler2D u_Bloom;

in vec2 inUV;
out vec4 frag_color;

void main() 
{
	vec4 colorA = texture(u_Scene, inUV);
	vec4 colorB = texture(u_Bloom, inUV);

	frag_color = 1.0 - (1.0 - colorA) * (1.0 - colorB);
}