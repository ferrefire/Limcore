#version 460

#extension GL_ARB_shading_language_include : require

layout(set = 0, binding = 1) uniform sampler2D texDiff;
layout(set = 0, binding = 2) uniform sampler2D texNorm;
layout(set = 0, binding = 3) uniform sampler2D texARM;

layout(location = 0) in vec3 worldNormal;
layout(location = 1) in vec2 worldCoordinate;
layout(location = 2) in vec3 worldPosition;
layout(location = 3) in vec3 viewPosition;

layout(location = 0) out vec4 pixelColor;

#include "lighting.glsl"
#include "sampling.glsl"

void main()
{
	vec3 color = texture(texDiff, worldCoordinate).rgb;
	vec3 weights = GetWeights(normalize(worldNormal), 1.0);
	vec3 normal = SampleNormal(texNorm, worldCoordinate, normalize(worldNormal));
	vec3 arm = texture(texARM, worldCoordinate).rgb;
	float roughness = arm.g;
	float metallic = arm.b;
	float ao = arm.r;

	PBRInput data;
	data.N = normal;
	data.V = normalize(viewPosition - worldPosition);
	data.L = normalize(vec3(0.2, 1, -0.4));
	data.albedo = color;
	data.metallic = metallic;
	data.roughness = roughness;
	data.lightColor = vec3(1.0, 0.9, 0.7) * 7.5;

	//vec3 diffuse = DiffuseLighting(normalize(worldNormal), normalize(vec3(0.2, 1, 0.4)), color);
	//vec3 diffuse = DiffuseLighting(normal, normalize(vec3(0.2, 1, 0.4)), color);
	vec3 diffuse = PBRLighting(data);

	//vec3 ambientDiffuse = 0.15 * color;
	//vec3 ambient = ambientDiffuse * ao;
	//diffuse += ambient;

	pixelColor = vec4(diffuse, 1.0);
}