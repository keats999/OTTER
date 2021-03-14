#version 420

layout(location = 0) in vec2 inUV;

out vec4 frag_color;

layout (binding = 0) uniform sampler2D s_screenTex;

uniform float u_Intensity;
uniform float u_Time;

float mod(float num1, float num2)
{
	return num1 - floor(num1 / num2) * num2;
}

float rand()
{
	//credit to appas at https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl for a psudeo random number generator base
	return abs(fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233)) * (mod(u_Time, 10.0) + 1.0)) * 43758.5453));
}

void main() 
{
	vec4 source = texture(s_screenTex, inUV);

	vec3 pixelColor = mix(vec3(0.0), source.rgb, round(rand()));

	frag_color.rgb = mix(source.rgb, pixelColor, u_Intensity);
    frag_color.a = source.a;
}