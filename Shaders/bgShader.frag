#version 330 core

out vec4 colour;

uniform vec3 bgColour;
in vec4 FragPosLightSpace;

uniform sampler2D shadowMap;

float ShadowCalculation()
{
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w; // ได้ค่า -1 ถึง 1 เเต่่ต้องเเปลงเป็น 0 ถึง 1 โดยการคูณ 0.5 เเล้วบอกด้วย 0.5
    projCoords = projCoords * 0.5 + 0.5;

    float closeDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // if(currentDepth > closestDepth){
    //     //in shadow
    //     return 1.0;
    // }
    // else return 0.0;
    float bias = 0.005; // ใส่มากัน shadow acne ภาพจะเป็นเงามีริ้วๆ
    // float shadow = currentDepth - bias > closeDepth? 1.0:0.0;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
    for(int y = -1; y <= 1; ++y)
    {
    float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
    shadow += currentDepth - bias > pcfDepth ? 0.7 : 0.0;
    }
    }
    shadow /= 9.0;
    return shadow;
}


void main()
{
    float shadow = ShadowCalculation();
    colour = vec4((1.0f - shadow)*bgColour, 1.0f);
}
