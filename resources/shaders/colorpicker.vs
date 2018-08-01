#version 300 es
precision highp float;

layout( location = 0) in vec3 aPos;
layout( location = 1) in vec2 aTexcoord;

layout(std140) uniform Common
{
	mat4 view;
	mat4 projection;
	mat4 invVP;
};


uniform mat4 model;

out vec2 vTexcoord;
out vec4 vWorldPos;

void main(void)
{
    vTexcoord = aTexcoord;
	vWorldPos = model * vec4(aPos.xyz, 1.0);
	vec4 pos = projection * view * vWorldPos;

    gl_Position = pos;
}