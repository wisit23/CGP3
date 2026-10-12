#version 330 core

out vec4 colour;
in vec4 vCol;
in vec2 TexCoord;
uniform sampler2D texture_data;
uniform sampler2D texture_data_2;
in vec3 Normal;
uniform vec3 lightColour;
uniform vec3 lightPos;
in vec3 FragPos;
uniform vec3 viewPos;
vec3 ambientLight(){
    float ambientStrength = 0.3f;
    vec3 ambient = lightColour * ambientStrength ;
    return ambient;
}

vec3 diffuseLight(){
    float diffuseStrength = 0.5f;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = lightColour * diff * diffuseStrength;
    return diffuse;
}

vec3 specularLight(){
    float specularStrength = 0.8f;
    float shininess = 64.0f;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    //Phong
    float spec = pow(max(dot(reflectDir, viewDir), 0.0f), shininess);

    //Blinn-Phong
    vec3 halfDir = normalize(lightDir + viewDir) / 2.0f;
    float spec2 = pow(max(dot(halfDir, norm), 0.0f), shininess);

    vec3 specular = lightColour * spec * specularStrength;
    return specular;
}

void main()
{
    // vec4 tex = texture(texture_data, TexCoord);
    // vec4 tex2 = texture(texture_data_2, TexCoord);
    // colour = mix(vCol, tex, tex.a);
    // colour = mix(tex, tex2, tex2.a);
    colour = texture(texture_data, TexCoord)*vec4(ambientLight() + diffuseLight() + specularLight(), 1.0f);
}
