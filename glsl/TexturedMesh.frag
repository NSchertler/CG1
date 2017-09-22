#version 330

in vec3 position;
in vec3 normal;
in vec2 texCoord;

#ifdef HAS_TESSELLATION
in vec3 tangent;
in vec3 bitangent;
#endif

out vec4 result;

uniform vec3 lightPosInViewSpace;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D displacementMap;

const vec3 constantAlbedo = vec3(0.6, 0.6, 0.6);
const vec3 lightColor = vec3(1, 1, 1);
const vec3 ambientLight = vec3(0.05, 0.05, 0.05);
const float shininess = 75;
const float specularity = 0.3;

const float gamma = 2.2;

uniform bool useAlbedoMap;
uniform bool useDisplacementMap;
uniform bool useNormalMap;

uniform float displacementScale;
const int steps = 16;

void calculateTangentSpace( vec3 N, vec3 p, vec2 uv, out mat3 tangentSpaceToViewSpace, out mat3 viewSpaceToTangentSpace )
{
#ifdef HAS_TESSELLATION
	tangentSpaceToViewSpace = mat3(tangent, bitangent, N);
#else
    // get derivatives of fragment position and texture coordinates with respect to screen position
    vec3 dpdx = dFdx( p );
    vec3 dpdy = dFdy( p );
    vec2 duvdx = dFdx( uv );
    vec2 duvdy = dFdy( uv );
 
	//Calculate dp/duv
	mat2 m = inverse(mat2(duvdx, duvdy));	
	mat2x3 TB = mat2x3(dpdx, dpdy) * m;
	tangentSpaceToViewSpace = mat3(TB[0], TB[1], N);

	//Calculate duv/dp = inverse(dp/duv)
    vec3 dpdyperp = cross( dpdy, N );
    vec3 dpdxperp = cross( N, dpdx );
	float invDet = 1 / dot(cross(dpdx, dpdy), N);
    vec3 T = invDet * ( dpdyperp * duvdx.x + dpdxperp * duvdy.x);
    vec3 B = invDet * ( dpdyperp * duvdx.y + dpdxperp * duvdy.y);
 
    viewSpaceToTangentSpace = transpose(mat3( T, B, N ));
#endif
}


void main(void)
{	
	vec2 textureCoordinate = texCoord;
	vec3 fragmentPosition = position;

	vec3 n = normalize(normal);
	vec3 dirToViewer = normalize(-fragmentPosition); //viewer is at the origin in camera space	
		
	mat3 tangentSpaceToViewSpace, viewSpaceToTangentSpace;
	calculateTangentSpace(n, fragmentPosition, textureCoordinate, tangentSpaceToViewSpace, viewSpaceToTangentSpace);

#ifndef HAS_TESSELLATION
	if(useDisplacementMap)
	{
		vec3 dirToViewerInTangentSpace = viewSpaceToTangentSpace * dirToViewer;

		//scale the direction such that after "steps/2" steps, we have reached a height (z-coordinate) of "displacementScale" with a small margin of 1.1
		dirToViewerInTangentSpace *= -1.1 * 2 * displacementScale / (dirToViewerInTangentSpace.z * steps);

		vec3 currentTexelPos = vec3(texCoord, 0) - steps / 2 * dirToViewerInTangentSpace; //z-coordinate is height 
		vec3 displacedTexelPos = currentTexelPos;
		float currentMapHeight = displacementScale * (2 * texture(displacementMap, currentTexelPos.xy).r - 1);
		for(int i = 0; i < steps; ++i)
		{
			vec3 newTexelPos = currentTexelPos + dirToViewerInTangentSpace;
			float newMapHeight = displacementScale * (2 * texture(displacementMap, newTexelPos.xy).r - 1);
			if(currentTexelPos.z > currentMapHeight && newTexelPos.z <= newMapHeight)
			{
				//interpolate to find intersection
				float t = (currentTexelPos.z - currentMapHeight) / (currentTexelPos.z - currentMapHeight - newTexelPos.z + newMapHeight);
				displacedTexelPos = currentTexelPos + t * (newTexelPos - currentTexelPos);
				break;
			}
			currentTexelPos = newTexelPos;
			currentMapHeight = newMapHeight;
		}
		//transform the displaced position back to view space
		fragmentPosition = position + tangentSpaceToViewSpace * (displacedTexelPos - vec3(texCoord, 0));
		textureCoordinate = displacedTexelPos.xy;
	}
#endif

	vec3 dirToLight = normalize(lightPosInViewSpace - fragmentPosition);

	if(useNormalMap)
	{
		vec3 mapNormal = texture(normalMap, textureCoordinate).xyz * 2 - 1;
		mapNormal.y *= -1;
		tangentSpaceToViewSpace[0] = normalize(tangentSpaceToViewSpace[0]);
		tangentSpaceToViewSpace[1] = normalize(tangentSpaceToViewSpace[1]);
		n = normalize(tangentSpaceToViewSpace * mapNormal);
	}

	//Now calculate lighting

	vec3 h = normalize(dirToViewer + dirToLight);
	float nDotL = dot(dirToLight, n);

	vec3 albedo = constantAlbedo;
	if(useAlbedoMap)
		albedo = pow(texture(albedoMap, textureCoordinate).xyz, vec3(gamma));

	vec3 ambient  = ambientLight * albedo;
	vec3 diffuse  = vec3(0, 0, 0);
	vec3 specular = vec3(0, 0, 0);
	if(nDotL > 0)
	{
		diffuse = nDotL * albedo  * lightColor;
		specular = specularity * pow(dot(h, n), shininess) * lightColor;	
	}
	
	result.rgb = ambient + diffuse + specular;

	result.a = 1;

	//Gamma correction	
	result.rgb = pow(result.rgb, vec3(1.0 / gamma));
}