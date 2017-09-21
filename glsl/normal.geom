#version 330

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in vec3 position[];
in vec3 normal[];

uniform mat4 modelViewProjection;
uniform float normalLength;

void main()
{
	gl_Position = modelViewProjection * vec4(position[0], 1);
	EmitVertex();
	gl_Position = modelViewProjection * vec4(position[0] + normalLength * normal[0], 1);
	EmitVertex();
}