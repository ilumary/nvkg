#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragUV;

struct ObjectData 
{
    mat4 transform;
    mat4 normalMatrix;
};

struct CameraData
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

struct LightData
{
    vec4 lightColor;
    vec3 position;
};

layout (std140, set = 0, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[1000];
} objectBuffer;

layout (set = 1, binding = 0) uniform GlobalData {
    CameraData cameraData;
    LightData lightData[10];
    int light_index;
} globalData;

void main() {

    ObjectData object = objectBuffer.objects[gl_BaseInstance];

    vec4 positionWorld = object.transform * vec4(position, 1.0);

    CameraData camera = globalData.cameraData;

    gl_Position = camera.projectionMatrix * camera.viewMatrix * positionWorld;
    fragNormalWorld = normalize(mat3(object.normalMatrix) * normal);
    fragPosWorld = positionWorld.xyz;
    fragColor = color;
    fragUV = uv;
}