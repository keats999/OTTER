#version 410

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

uniform sampler2D s_Diffuse;
uniform sampler2D s_Diffuse2;
uniform sampler2D s_Specular;

uniform vec3  u_CamPos;

out vec4 frag_color;


// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {
	// Get the albedo from the diffuse / albedo map
	vec4 textureColor = texture(s_Diffuse, inUV);

	vec3 result = inColor * textureColor.rgb;

	frag_color = vec4(result, textureColor.a);
}