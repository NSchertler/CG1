#version 330
layout(location = 0) in  vec3 vPosition;
layout(location = 1) in  vec3 vNormal;
layout(location = 2) in  vec2 vTexCoord;

out vec4 color;

uniform mat4 modelViewProjection;

void main()
{
	gl_Position = modelViewProjection * vec4(vPosition, 1);	
	color = vec4(0.8, 0.8, 0.8, 1);
}
