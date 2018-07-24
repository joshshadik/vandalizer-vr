#version 300 es
precision highp float;

in vec2 vTexcoord;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBinormal;
in vec4 vWorldPos;
in vec4 vScreenPos;
in vec4 vToolPos;

layout(std140)  uniform Common
{
	mat4 view;
	mat4 projection;
	mat4 invVP;
};


uniform mat4 model;


uniform sampler2D uDiffuse;
uniform sampler2D uNormalTex;
uniform sampler2D uOcclusionTex;



layout(location = 0) out vec4 color;

vec3 getNormal()
{
	vec3 n = texture(uNormalTex, vTexcoord.st).xyz;
	n = normalize(n * 2.0 - 1.0);
	n = n.x * vTangent.xyz + n.y * vBinormal.xyz + n.z * vNormal.xyz;

	return n;
}


void main(void) {
	vec4 col = texture(uDiffuse, vTexcoord.st);
	col.a = 1.0;

	vec3 scrPos = vScreenPos.xyz / vScreenPos.w;
	vec3 toolPos = vToolPos.xyz / vToolPos.w;

	//vec2 scrOff = scrPos.xy - vec2(0.5);
	if( length(toolPos.xy) < 0.025 )
	{
		col.rgb = vec3(1.0, 0.5, 0.5) * 0.2 + col.rgb * 0.8;
	}

	//col.rgb = vWorldPos.xyz;
	
    color = col;
}