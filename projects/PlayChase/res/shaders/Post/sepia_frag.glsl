#version 420

layout(location = 0) in vec2 inUV;

out vec4 frag_color;

layout (binding = 0) uniform sampler2D s_screenTex;

uniform float u_Intensity=0.6;

void main() 
{
	vec4 source = texture(s_screenTex, inUV);

	vec3 sepiaColor;
	sepiaColor.r= 0.393 * source.r + 0.769 * source.g + 0.189* source.b;
	sepiaColor.g= 0.349 * source.r + 0.686 * source.g + 0.168* source.b;
	sepiaColor.b= 0.272 * source.r + 0.534 * source.g + 0.131* source.b;

	frag_color.rgb = mix(source.rgb,sepiaColor.rgb,u_Intensity);
	frag_color.a = source.a;
}