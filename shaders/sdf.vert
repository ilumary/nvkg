#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

layout(push_constant) uniform Push {
	float outlineWidth;
	float outlineEnabled;
    float textThickness;
    vec2 textPosition;
    vec2 textScale;
    float textRotation; //TODO implement
} push;

void main() {
	outUV = inUV;
	outColor = inColor;
	gl_Position = vec4((inPos.xy * push.textScale) + push.textPosition, 0.0, 1.0);
}