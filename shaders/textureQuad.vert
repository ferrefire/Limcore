#version 460

#extension GL_ARB_shading_language_include : require

layout(set = 0, binding = 0) uniform Variables
{
	mat4 model;
	mat4 view;
	mat4 projection;
} variables;

layout(location = 0) in vec3 localPosition;
layout(location = 1) in vec2 localCoordinate;

layout(location = 0) out vec2 worldCoordinate;

void main()
{
	worldCoordinate = localCoordinate;
	gl_Position = variables.projection * variables.view * variables.model * vec4(localPosition, 1.0);
}