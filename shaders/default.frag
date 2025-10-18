#version 460

#extension GL_ARB_shading_language_include : require

layout(set = 0, binding = 0) uniform Variables
{
	mat4 view;
	mat4 projection;
	vec4 viewPosition;
	vec4 lightDirection;
} variables;

layout(set = 1, binding = 0) uniform sampler2D textures[3];

layout(location = 0) in vec3 worldNormal;
layout(location = 1) in vec2 worldCoordinate;
layout(location = 2) in vec3 worldPosition;
layout(location = 3) in vec3 viewPosition;

layout(location = 0) out vec4 pixelColor;

#include "lighting.glsl"
#include "sampling.glsl"

void main()
{
	vec3 color = texture(textures[0], worldCoordinate).rgb;
	vec3 weights = GetWeights(normalize(worldNormal), 1.0);
	vec3 normal = SampleNormal(textures[1], worldCoordinate, normalize(worldNormal)).rgb;
	vec3 arm = texture(textures[2], worldCoordinate).rgb;
	float roughness = arm.g;
	float metallic = arm.b;
	float ao = arm.r;

	PBRInput data;
	data.N = normal;
	data.V = normalize(viewPosition - worldPosition);
	data.L = variables.lightDirection.xyz;
	data.albedo = color;
	data.metallic = metallic;
	data.roughness = roughness;
	data.lightColor = vec3(1.0, 0.9, 0.7) * 4;

	vec3 diffuse = PBRLighting(data);

	vec3 ambientDiffuse = 0.1 * color * vec3(1.0, 0.9, 0.7);
	vec3 ambient = ambientDiffuse * ao;
	diffuse += ambient;

	pixelColor = vec4(diffuse, 1.0);
	//pixelColor = vec4(normalize(UnpackNormal(texture(textures[1], worldCoordinate), 1.0) * 0.5 + 0.5), 1.0);
	//pixelColor = vec4(UnpackNormal(texture(textures[1], worldCoordinate), 1.0), 1.0);
	//pixelColor = vec4((normal * 0.5 + 0.5), 1.0);
	//pixelColor = vec4(worldCoordinate, 1.0, 1.0);
	//pixelColor = vec4(texture(textures[1], worldCoordinate).rgb, 1.0);

	//if (worldPosition.z > 0 && abs(worldPosition.x) < 0.1) pixelColor = vec4(0, 0, 1, 1);
	//if (worldPosition.x > 0 && abs(worldPosition.z) < 0.1) pixelColor = vec4(0, 1, 0, 1);
}