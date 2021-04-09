#version 420

layout(location = 0) in vec2 inUV;

out vec4 frag_color;

layout (binding = 0) uniform sampler2D s_screenTex;


uniform float u_Intensity;
uniform float u_InnerRadius;
uniform float u_OuterRadius;


void main() 
{
	vec3 vignetteColor = vec3(1.0, 0.0, 0.0);
	vec4 source = texture(s_screenTex, inUV);
	vec2 screenSize = textureSize(s_screenTex, 0).xy;
    vec2 relativePosition = gl_FragCoord.xy / screenSize - .5;
   // relativePosition.y *= screenSize.x / screenSize.y;
    float len = length(relativePosition);
	float vignetteOpacity = smoothstep(u_InnerRadius, u_OuterRadius, len) * u_Intensity;
	frag_color.rgb = mix(source.rgb, vignetteColor, vignetteOpacity);
	frag_color.a = source.a;
}