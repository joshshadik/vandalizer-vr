#version 300 es
precision highp float;

in vec2 vTexcoord;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBinormal;
in vec4 vWorldPos;

layout(std140)  uniform Common
{
	mat4 view;
	mat4 projection;
	mat4 invVP;
};

uniform PBR
{
	vec4 diffuseFactor;
	float metallicFactor;
	float roughnessFactor;
};


uniform mat4 model;
uniform vec4 diffuseColor;

uniform sampler2D uDiffuse;
uniform sampler2D uNormalTex;
uniform sampler2D uOcclusionTex;

uniform vec2 uDiffuse_TexSize;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 position;
layout(location = 2) out vec4 normal;
layout(location = 3) out vec4 metalRoughOcc;

vec4 SRGBtoLinear(vec4 srgbIn )
{
	vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
	vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
	return vec4(linOut,srgbIn.w);
}

vec3 getNormal()
{
	vec3 n = texture(uNormalTex, vTexcoord.st).xyz;
	n = normalize(n * 2.0 - 1.0);
	n = n.x * vTangent.xyz + n.y * vBinormal.xyz + n.z * vNormal.xyz;

	return n;
}


void main(void) {
	vec4 col = vec4(0.0);
	col.a = 0.0;

	for( int x = -1; x <= 1; ++x )
	{
		for( int y = -1; y <= 1; ++y )
		{
			col += texture(uDiffuse, vTexcoord.st + uDiffuse_TexSize.st * vec2(x, y));
		}
	}

	if( col.a > 1.0 )
	{
		col.rgb /= col.a;
	}


    //vec4 col = texture(uDiffuse, vTexcoord.st); // vec4(vTexcoord.st, 0.0, 1.0); //
	col.a = 1.0;
	col *= diffuseFactor;

	vec3 n = getNormal();
	// float d = dot(n, normalize(vec3(0.0, 1.0, 1.0)));

	// col.rgb *= d;

	vec4 occ = texture(uOcclusionTex, vTexcoord.st);
	//col.rgb = col.rgb * occ.r;
	//col.rgb = col.rgb * 0.5 * ( 1.0 +  occ.r );
	
    color = col;
	position = vWorldPos;
	normal = vec4(n, 1.0);
	metalRoughOcc = vec4(metallicFactor * occ.g, roughnessFactor * occ.b, occ.r, 1.0);
}