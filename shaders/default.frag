#version 460

#extension GL_ARB_shading_language_include : require

layout(set = 0, binding = 1) uniform sampler2D tex;

layout(location = 0) in vec3 worldNormal;
layout(location = 1) in vec2 worldCoordinate;

layout(location = 0) out vec4 pixelColor;

#include "lighting.glsl"

float rand(vec2 seed)
{
	return (fract(sin(dot(seed.xy, vec2(12.9898, 78.233))) * 43758.5453));
}

void main()
{
	//int primitiveID = gl_PrimitiveID;
	//float r = rand(vec2(primitiveID));
	//float g = rand(vec2(r));
	//float b = rand(vec2(g));
	//vec3 color = vec3(r, g, b);
	//pixelColor = vec4(color, 1.0);

	//vec3 color = texture(tex, worldCoordinate).rgb;
	//pixelColor = vec4(texColor, 1.0);
	pixelColor = vec4(worldCoordinate.x, worldCoordinate.y, 1.0, 1.0);

	//vec3 diffuse = DiffuseLighting(normalize(worldNormal), normalize(vec3(0.2, 1, 0.4)), vec3(0.25, 0.0, 0.75));
	//vec3 diffuse = DiffuseLighting(normalize(worldNormal), normalize(vec3(0.2, 1, 0.4)), color);
	//pixelColor = vec4(diffuse, 1.0);
}