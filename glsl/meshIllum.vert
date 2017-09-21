#version 330
layout(location = 0) in  vec3 vPosition;
layout(location = 1) in  vec3 vNormal;
layout(location = 2) in  vec2 vTexCoord;

uniform mat4 modelViewProjection;
uniform mat4 modelView;

out vec3 position;
out vec3 normal;

void main()
{
	gl_Position = modelViewProjection * vec4(vPosition, 1);	

	position = (modelView * vec4(vPosition, 1)).xyz;
	normal = (modelView * vec4(vNormal, 0)).xyz;
}
