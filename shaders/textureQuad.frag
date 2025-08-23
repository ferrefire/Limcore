#version 460

#extension GL_ARB_shading_language_include : require

layout(set = 0, binding = 1) uniform sampler2D tex;

layout(location = 0) in vec2 worldCoordinate;

layout(location = 0) out vec4 pixelColor;

void main()
{
	vec3 color = texture(tex, worldCoordinate).rgb;
	pixelColor = vec4(color, 1.0);
}