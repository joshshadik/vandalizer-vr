#version 300 es
precision highp float;

in vec2 vTexcoord;
in vec4 vWorldPos;

layout(std140)  uniform Common
{
	mat4 view;
	mat4 projection;
	mat4 invVP;
};


uniform mat4 model;
uniform vec2 pickerLocation;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 position;
layout(location = 2) out vec4 normal;
layout(location = 3) out vec4 metalRoughOcc;

const float PI = 3.1457;

vec3 hue(float h)
{
    float r = abs(h * 6.0 - 3.0) - 1.0;
    float g = 2.0 - abs(h * 6.0 - 2.0);
    float b = 2.0 - abs(h * 6.0 - 4.0);
    return clamp(vec3(r,g,b), 0.0, 1.0);
}

vec3 hsv2rgb(vec3 hsv)
{
    return vec3(((hue(hsv.r) - vec3(1.0)) * hsv.g + vec3(1.0)) * hsv.b);
}

void main(void) {
    

    vec2 coord = vTexcoord - vec2(0.5);
    coord.x = -coord.x;

    coord *= 2.0;

    float rad = length(coord);
    float theta = atan(coord.y, coord.x);
    theta = (theta + PI) / (2.0 * PI);

    vec4 col = vec4(0.0);
    col.rgb = hsv2rgb(vec3(theta, 1.0, rad));
    col.a = smoothstep( 1.0, 0.99, rad);

    vec2 off = coord - pickerLocation;
    float dist = length(off);
    if( dist < 0.1 && dist > 0.07 )
    {
        col.rgb = vec3(1.0);
        col.a = 1.0;
    }


	
    color = col;
	position = vWorldPos;
	normal = vec4(0.0);
	metalRoughOcc = vec4(0.0);
}