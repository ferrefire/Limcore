#ifndef LIGHTING_INCLUDED
#define LIGHTING_INCLUDED

vec3 DiffuseLighting(vec3 normal, vec3 lightDirection, vec3 color)
{
	float product = dot(normal, lightDirection);
	float intensity = max(0.1, product);
	vec3 result = color * intensity;

	return (result);
}

vec3 ToNonLinear(vec3 linearColor)
{
	vec3 cutoff = step(vec3(0.0031308), linearColor);
	vec3 lower = linearColor * 12.92;
	vec3 higher = 1.055 * pow(linearColor, vec3(1.0 / 2.4)) - 0.055;

	return (mix(lower, higher, cutoff));
}

vec3 Normalize(vec3 vec)
{
	float total = abs(vec.x) + abs(vec.y) + abs(vec.z);
	vec3 result = vec / total;

	return (vec);
}

#endif