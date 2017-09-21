#version 330

in vec3 position;
in vec3 normal;

out vec4 result;

uniform vec3 lightPosInViewSpace;

uniform vec3 albedo;
uniform vec3 lightColor;
uniform vec3 ambientLight;
uniform float shininess;
uniform float specularity;

void main(void)
{	
	vec3 n = normalize(normal);
	vec3 dirToViewer = normalize(-position); //viewer is at the origin in camera space
	vec3 dirToLight = lightPosInViewSpace - position;
	float distanceFromLight = length(dirToLight);
	vec3 incomingLight = lightColor / (distanceFromLight * distanceFromLight);
	dirToLight = normalize(dirToLight);

	vec3 h = normalize(dirToViewer + dirToLight);

	float nDotL = dot(dirToLight, n);

	vec3 ambient  = ambientLight * albedo;
	vec3 diffuse  = vec3(0, 0, 0);
	vec3 specular = vec3(0, 0, 0);
	if(nDotL > 0)
	{
		diffuse = nDotL * albedo  * incomingLight;
		specular = specularity * pow(dot(h, n), shininess) * incomingLight;	
	}
	
	result.rgb = ambient + diffuse + specular;

	result.a = 1;

	//Gamma correction
	const float gamma = 2.2;
	result.rgb = pow(result.rgb, vec3(1.0 / gamma));
}