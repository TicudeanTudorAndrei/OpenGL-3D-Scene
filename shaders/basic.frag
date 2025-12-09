#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

//positional lights coords
uniform vec3 lightPosition;
uniform vec3 lightPosition2;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

//bools
uniform bool sunOn;
uniform bool lampOn;
uniform bool isShiny;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

//added function to factor in the positional light
void computePosLight(vec3 lightPosition) {
    float distance = length(lightPosition - fPosition);
    float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));

    diffuse += attenuation * diffuse * lightColor;
    ambient += attenuation * ambient * lightColor;
    specular += attenuation * specular * lightColor;
}

void main() 
{	
    computeDirLight();
    if(lampOn) {
	computePosLight(lightPosition);
	computePosLight(lightPosition2);
    }
	
    // created variables for texture colors
    vec4 diffuseTexColor = texture(diffuseTexture, fTexCoords);
    vec4 specularTexColor = texture(specularTexture, fTexCoords);

    // removed black from border texture and leaves
    if (all(lessThan(diffuseTexColor.rgb, vec3(0.001)))) {
        discard;
    }

    //compute final vertex color
    vec3 color;

    //added light on - off 
    if(sunOn) {
	    if(isShiny)
		color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular, 1.0f);	//add white point
	    else            
		color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);	//remove white point
    }
    else
	    color = texture(diffuseTexture, fTexCoords).rgb;
    fColor = vec4(color, 1.0f);
}
