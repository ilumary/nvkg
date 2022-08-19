#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct CameraData {
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

struct LightData {
    vec4 lightColor;
    vec4 ambientLightColor;
    vec3 position;
};

layout (set = 0, binding = 0) uniform GlobalData {
    CameraData cameraData;
    LightData lightData[10];
    int light_index;
} globalData;

layout(push_constant) uniform Push {
  vec4 position;
  vec4 color;
  float radius;
} push;

//vec4 lightColor = globalData.lightData.lightColor;

void main() {
    float dis = sqrt(dot(fragOffset, fragOffset));

    if (dis >= 1.0) {
        discard;
    }

    vec3 lightcolor = vec3(push.color.xyz);
    
    outColor = vec4(lightcolor, 1.0);
}