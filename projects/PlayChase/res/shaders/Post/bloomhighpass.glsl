#version 420
layout(location = 0) in vec2 inUV;

layout(binding = 0) uniform sampler2D s_screenTex; //Source image
uniform float u_PixelSize; //1.0 / Window_Height
uniform float u_Threshold;

out vec4 frag_color;

void main() 
{
	vec4 color = texture(s_screenTex, inUV);
	
	float luminance = (color.r + color.g + color.b) / 3.0;
	
	if (luminance > u_Threshold) 
	{
		frag_color = color;
	}
	else
	{
		frag_color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}