#version 410

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outUV;

uniform mat4 u_ModelViewProjection;
uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat3 u_NormalMatrix;
uniform vec3 u_LightPos;
uniform mat4 u_UIMatrix;
uniform mat3 u_EnvironmentRotation;

void main() {

	vec4 pos = u_UIMatrix * vec4(inPosition, 1.0);
    gl_Position = pos.xyww;

    // Normals
    outNormal = inPosition;

}

