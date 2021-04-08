#version 420

layout(location = 0) in vec2 inUV;

struct DirectionalLight
{
	//Light direction (defaults to down, to the left, and a little forward)
	vec4 _lightDirection;

	//Generic Light controls
	vec4 _lightCol;

	//Ambience controls
	vec4 _ambientCol;
	float _ambientPow;
	
	//Power controls
	float _lightAmbientPow;
	float _lightSpecularPow;
	
	float _shadowBias;
};
 
 struct PointLight {    
	vec4 _lightPos;
	vec4 _lightCol;
	vec4 _ambientCol;

	float _lightLinearFalloff;
	float _lightQuadraticFalloff;
	float _ambientPow;
	float _lightAmbientPow;
	float _lightSpecularPow;
};  

//#define NR_POINT_LIGHTS 0  
//uniform PointLight pointLights[NR_POINT_LIGHTS];

layout (std140, binding = 0) uniform u_Lights
{
	DirectionalLight sun;
};

layout (std140, binding = 1) uniform u_pointLights
{
	PointLight playerLight;
};

layout (binding = 30) uniform sampler2D s_ShadowMap;

layout (binding = 0) uniform sampler2D s_albedoTex;
layout (binding = 1) uniform sampler2D s_normalsTex;
layout (binding = 2) uniform sampler2D s_specularTex;
layout (binding = 3) uniform sampler2D s_positionTex;

layout (binding = 4) uniform sampler2D s_lightAccumTex;

uniform float u_Emission;
uniform mat4 u_LightSpaceMatrix;
uniform vec3 u_CamPos;

out vec4 frag_color;

float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
	//Perspective division
	vec3 projectionCoordinates = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
	//Transform into a [0,1] range
	projectionCoordinates = projectionCoordinates * 0.5 + 0.5;
	
	//Get the closest depth value from light's perspective (using our 0-1 range)
	float closestDepth = texture(s_ShadowMap, projectionCoordinates.xy).r;

	//Get the current depth according to the light
	float currentDepth = projectionCoordinates.z;

	//Check whether there's a shadow
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	//Return the value
	return shadow;
}

void main() {
	//Albedo
	vec4 textureColor = texture(s_albedoTex, inUV);
	//Normals
	vec3 inNormal = (normalize(texture(s_normalsTex, inUV).rgb) * 2.0) - 1.0;
	//Specular
	float texSpec = texture(s_specularTex, inUV).r;
	//Positions
	vec3 fragPos = texture(s_positionTex, inUV).rgb;


	// Diffuse
	vec3 N = normalize(inNormal);
	vec3 lightDir = normalize(-sun._lightDirection.xyz);
	float dif = max(dot(N, lightDir), 0.0);
	vec3 diffuse = dif * sun._lightCol.xyz;// add diffuse intensity

	// Specular
	vec3 viewDir  = normalize(u_CamPos - fragPos);
	vec3 h        = normalize(lightDir + viewDir);

	float spec = pow(max(dot(N, h), 0.0), 4.0); // Shininess coefficient (can be a uniform)
	vec3 specular = sun._lightSpecularPow * texSpec * spec * sun._lightCol.xyz; // Can also use a specular color

	vec4 fragPosLightSpace = u_LightSpaceMatrix * vec4(fragPos, 1.0);
	float shadow = ShadowCalculation(fragPosLightSpace, sun._shadowBias);

	vec3 pointTotal = vec3(0.0, 0.0, 0.0);

	vec3 lightDirP = normalize(playerLight._lightPos.xyz - fragPos);
		// diffuse shading
	float difP = max(dot(N, lightDirP), 0.0);
		// specular shading
	vec3 reflectDir = reflect(-lightDir, N);
	float specP = pow(max(dot(viewDir, reflectDir), 0.0), 1.0);
		//float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		// attenuation
	float dist   = length(playerLight._lightPos.xyz - fragPos);
	float attenuation = 1.0 / (playerLight._ambientPow + playerLight._lightLinearFalloff * dist + 
  				 playerLight._lightQuadraticFalloff * (dist * dist));    
	// combine results
	vec3 ambientP  = playerLight._ambientCol.xyz * playerLight._lightAmbientPow * attenuation * textureColor.xyz;
	vec3 diffuseP  = playerLight._lightCol.xyz * difP * attenuation * textureColor.xyz;
	vec3 specularP = playerLight._lightCol.xyz * specP * texSpec * playerLight._lightSpecularPow * attenuation;
	pointTotal += ambientP + diffuseP + specularP;

	vec3 result = (
		(sun._ambientPow * sun._ambientCol.xyz) + // global ambient light
		(1.0 - shadow) * //Shadow value
		(diffuse + specular + pointTotal));

	//result = mix(result, textureColor.rgb, u_Emission);
	if(textureColor.a < 0.31){
		result = vec3(0.0, 0.0, 0.0);
	}
	frag_color = vec4(result, textureColor.a);
}