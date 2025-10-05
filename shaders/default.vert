#version 460

#extension GL_ARB_shading_language_include : require

layout(set = 0, binding = 0) uniform Variables
{
	mat4 view;
	mat4 projection;
	vec4 viewPosition;
	vec4 lightDirection;
} variables;

layout(set = 2, binding = 0) uniform models { mat4 model; } object;

layout(location = 0) in vec3 localPosition;
layout(location = 1) in vec3 localNormal;
layout(location = 2) in vec2 localCoordinate;

layout(location = 0) out vec3 worldNormal;
layout(location = 1) out vec2 worldCoordinate;
layout(location = 2) out vec3 worldPosition;
layout(location = 3) out vec3 viewPosition;

void main()
{
	worldNormal = (object.model * vec4(localNormal, 0.0)).xyz;
	worldPosition = (object.model * vec4(localPosition, 1.0)).xyz;
	worldCoordinate = localCoordinate;
	viewPosition = variables.viewPosition.xyz;
	gl_Position = variables.projection * variables.view * object.model * vec4(localPosition, 1.0);
}