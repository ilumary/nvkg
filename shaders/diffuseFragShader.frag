#version 460

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragUV;

layout(set = 2, binding = 0) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

struct CameraData {
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

struct LightData {
    vec4 lightColor;
    vec4 ambientLightColor;
    vec3 position;
};

layout (set = 1, binding = 0) uniform GlobalData {
    CameraData cameraData;
    LightData lightData[10];
    int light_index;
} globalData;

//LightData lightData = globalData.lightData;

void main() {
    vec3 diffuseLight = vec3(0.f, 0.f, 0.f);
    vec3 surfaceNormal = normalize(fragNormalWorld);

    for (int i = 0; i < globalData.light_index; i++) {
        LightData light = globalData.lightData[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
        float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
        vec3 intensity = light.lightColor.xyz * light.lightColor.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;
    }
    
    //outColor = vec4(diffuseLight * fragColor, 1.0);
    outColor = texture(texSampler, fragUV) + vec4(diffuseLight * fragColor, 1.0);

    /*vec3 directionToLight = lightData.position - fragPosWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared

    vec3 lightColor = lightData.lightColor.xyz * lightData.lightColor.w * attenuation;
    vec3 ambientLight = lightData.ambientLightColor.xyz * lightData.ambientLightColor.w;
    vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);
    outColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);*/
}