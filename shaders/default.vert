#version 460

#extension GL_ARB_shading_language_include : require

layout(set = 0, binding = 0) uniform Variables
{
	mat4 model;
	mat4 view;
	mat4 projection;
} variables;

layout(location = 0) in vec3 localPosition;
layout(location = 1) in vec3 localNormal;
layout(location = 2) in vec2 localCoordinate;

layout(location = 0) out vec3 worldNormal;
layout(location = 1) out vec2 worldCoordinate;

void main()
{
	worldNormal = (variables.model * vec4(localNormal, 0.0)).xyz;
	worldCoordinate = localCoordinate;
	gl_Position = variables.projection * variables.view * variables.model * vec4(localPosition, 1.0);
}