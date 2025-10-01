#version 460

#extension GL_ARB_shading_language_include : require

layout(set = 0, binding = 1) uniform sampler2D texDiff;

layout(location = 0) in vec3 worldNormal;
layout(location = 1) in vec2 worldCoordinate;
layout(location = 2) in vec3 worldPosition;
layout(location = 3) in vec3 viewPosition;

layout(location = 0) out vec4 pixelColor;

#include "lighting.glsl"

void main()
{
	vec3 color = texture(texDiff, worldCoordinate).rgb;

	vec3 diffuse = DiffuseLighting(normalize(worldNormal), normalize(vec3(0.2, 1, -0.4)), color);

	pixelColor = vec4(diffuse, 1.0);
}