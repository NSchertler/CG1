#version 330
layout(location = 0) in  vec3 vPosition;
layout(location = 1) in  vec3 vNormal;

out vec3 position;
out vec3 normal;

void main()
{
	position = vPosition;
	normal = vNormal;
}
