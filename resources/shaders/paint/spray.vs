#version 300 es
precision highp float;

layout( location = 0) in vec3 aPos;
layout( location = 1) in vec2 aTexcoord;
layout( location = 2) in vec3 aNormal;
layout( location = 3) in vec4 aTangent;

layout(std140) uniform Common
{
	mat4 view;
	mat4 projection;
	mat4 invVP;
};


uniform mat4 model;
uniform mat4 toolVP;

out vec2 vTexcoord;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBinormal;
out vec4 vWorldPos;
out vec4 vScreenPos;
out vec4 vToolPos;

void main(void)
{
    vTexcoord = aTexcoord;
	vNormal = normalize((model * vec4(aNormal.xyz, 0.0)).xyz);
	vTangent = normalize((model * vec4(aTangent.xyz, 0.0)).xyz);
	vBinormal = cross(vNormal, vTangent) * aTangent.w;

	vec4 wPos = model * vec4(aPos.xyz, 1.0);
	vWorldPos = wPos;
	vScreenPos = projection * view * wPos;
	vToolPos = toolVP * wPos;

    gl_Position = vec4(vTexcoord * 2.0 - 1.0, 0.0, 1.0);
}