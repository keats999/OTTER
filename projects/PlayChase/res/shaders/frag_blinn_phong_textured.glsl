#version 410

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

uniform sampler2D s_Diffuse;
uniform sampler2D s_Diffuse2;
uniform sampler2D s_Specular;

uniform vec3  u_AmbientCol;
uniform float u_AmbientStrength;

uniform vec3  u_LightPos;
uniform vec3  u_LightCol;
uniform float u_AmbientLightStrength;
uniform float u_SpecularLightStrength;
uniform float u_Shininess;
// NEW in week 7, see https://learnopengl.com/Lighting/Light-casters for a good reference on how this all works, or
// https://developer.valvesoftware.com/wiki/Constant-Linear-Quadratic_Falloff
uniform float u_LightAttenuationConstant;
uniform float u_LightAttenuationLinear;
uniform float u_LightAttenuationQuadratic;

uniform float u_TextureMix;

uniform vec3  u_CamPos;

uniform int u_Mode;
uniform int u_Textures;

//cel shading
uniform float lightIntensity = 20.0;
const int bands = 5;
const float scaleFactor = 1.0/bands;

out vec4 frag_color;

uniform sampler2D s_RampTexture;
//Ramp lighting explained by 
//https://webglfundamentals.org/webgl/lessons/webgl-ramp-textures.html
vec4 RampCalc(float modify)
{
    float u = modify * 0.5 + 0.5;
    vec2 uv = vec2(u, 0.5);

    vec4 rampColor = texture(s_RampTexture, uv);

    return rampColor;
}

// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {
	// Lecture 5
	vec3 ambient = u_AmbientLightStrength * u_LightCol;

	// Diffuse
	vec3 N = normalize(inNormal);
	vec3 lightDir = normalize(u_LightPos - inPos);

	float dif = max(dot(N, lightDir), 0.0);
	vec3 diffuse = dif * u_LightCol;// add diffuse intensity

	//Attenuation
	float dist = length(u_LightPos - inPos);
	float attenuation = 1.0f / (
		u_LightAttenuationConstant + 
		u_LightAttenuationLinear * dist +
		u_LightAttenuationQuadratic * dist * dist);

	// Specular
	vec3 viewDir  = normalize(u_CamPos - inPos);
	vec3 h        = normalize(lightDir + viewDir);

	// Get the specular power from the specular map
	float texSpec = texture(s_Specular, inUV).x;
	float spec = pow(max(dot(N, h), 0.0), u_Shininess); // Shininess coefficient (can be a uniform)
	vec3 specular = u_SpecularLightStrength * (texSpec * u_Textures) * spec * u_LightCol; // Can also use a specular color

	// Get the albedo from the diffuse / albedo map
	vec4 textureColor1 = texture(s_Diffuse, inUV);
	vec4 textureColor2 = texture(s_Diffuse2, inUV);
	vec4 textureColor = mix(textureColor1, textureColor2, u_TextureMix);

	vec3 result = inColor * textureColor.rgb;
	if (u_Textures == 0)
	{
		result = vec3(1.0, 1.0, 1.0);
	}

	if(u_Mode == 2)
	{
		result = (
		(u_AmbientCol * u_AmbientStrength) + // global ambient light
		(ambient) * attenuation // light factors from our single light
		) * result; // Object color
	}
	else if(u_Mode == 3)
	{
		result = (
		(diffuse + specular) * attenuation // light factors from our single light
		) * result; // Object color
	}
	else if(u_Mode == 4)
	{
		//Cel Shading
		diffuse = floor(diffuse * bands) * scaleFactor;

		//Outline
		float edge=(dot(viewDir,N) < 0.2)? 0.0 : 1.0;
		
		result = (
		(u_AmbientCol * u_AmbientStrength) + // global ambient light
		(ambient + diffuse + specular) * edge * attenuation // light factors from our single light
		) * result; // Object color
	}
	else if(u_Mode == 5)
	{
		float dif = max(dot(N, lightDir), 0.0);
		vec4 rampColor = RampCalc(dif);
		result = (
        (u_AmbientCol * u_AmbientStrength) + // global ambient light
        (ambient + (diffuse*rampColor.rgb) + specular) * attenuation // light factors from our single light
        ) * result *rampColor.rgb;
	}
	else if(u_Mode == 6)
	{
		float spec = pow(max(dot(N, h), 0.0), u_Shininess);
		vec4 rampColor = RampCalc(spec);
		result = (
        (u_AmbientCol * u_AmbientStrength) + // global ambient light
        (ambient + diffuse + (specular*rampColor.rgb)) * attenuation // light factors from our single light
        ) * result *rampColor.rgb;
	}
	else
	{
		result = (
		(u_AmbientCol * u_AmbientStrength) + // global ambient light
		(ambient + diffuse + specular) * attenuation // light factors from our single light
		) * result; // Object color	
	}

	frag_color = vec4(result, textureColor.a);
}