#version 330

in vec3 position;
in vec3 normal;

out vec4 result;

uniform vec3 lightPosInViewSpace;

uniform vec3 albedo;
uniform vec3 lightColor;
uniform vec3 ambientLight;
uniform float roughness;

void main(void)
{
	vec3 n = normalize(normal);
	vec3 dirToViewer = normalize(-position); //viewer is at the origin in camera space
	vec3 dirToLight = lightPosInViewSpace - position;
	float distanceFromLight = length(dirToLight);
	vec3 incomingLight = lightColor / (distanceFromLight * distanceFromLight);
	dirToLight = normalize(dirToLight);	

	vec3 ambient = ambientLight * albedo;	

	float roughnessSquared = roughness * roughness;
	//formulas from https://en.wikipedia.org/wiki/Oren%E2%80%93Nayar_reflectance_model
	float A = 1 - 0.5 * roughnessSquared / (roughnessSquared + 0.33);
	float B = 0.45 * roughnessSquared / (roughnessSquared + 0.09);
	
	float thetaI = acos(dot(n, dirToLight));
	float thetaR = acos(dot(n, dirToViewer));
	float alpha = max(thetaI, thetaR);
	float beta = min(thetaI, thetaR);

	vec3 dirToViewerProjected = normalize(dirToViewer - dot(dirToViewer, n) * n);
	vec3 dirToLightProjected  = normalize(dirToLight  - dot(dirToLight , n) * n);

	vec3 diffuse = max(0.0, dot(dirToLight, n)) * albedo  * incomingLight
		 * (A + (B * dot(dirToViewerProjected, dirToLightProjected) * sin(alpha) * tan(beta)));

	result.rgb = ambient + diffuse;

	result.a = 1;

	//Gamma correction
	const float gamma = 2.2;
	result.rgb = pow(result.rgb, vec3(1.0 / gamma));
}