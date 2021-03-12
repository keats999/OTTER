#version 420

layout(location = 0) in vec2 inUV;

out vec4 frag_color;

layout (binding = 0) uniform sampler2D s_screenTex;

uniform float u_PercentOfPixels;

void main() 
{
    vec2 texSize = textureSize(s_screenTex, 0).xy;
    vec2 pixelatedTexSize = texSize * u_PercentOfPixels;
    vec4 result = texture(s_screenTex, (vec2(floor((gl_FragCoord.xy / (texSize / pixelatedTexSize)).x), floor((gl_FragCoord.xy / (texSize / pixelatedTexSize)).y)) * (texSize / pixelatedTexSize)) / texSize);

	frag_color = result;
}