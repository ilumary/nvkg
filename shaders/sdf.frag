#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;

layout (binding = 1) uniform sampler2D samplerColor;

layout(push_constant) uniform Push {
	float outlineWidth;
	float outlineEnabled;
} push;

layout (location = 0) out vec4 outFragColor;

void main() {
    float distance = texture(samplerColor, inUV).a * 0.85; //externalise bold factor
    float smoothWidth = fwidth(distance);	
    float alpha = smoothstep(0.5 - smoothWidth, 0.5 + smoothWidth, distance);
	vec3 rgb = vec3(alpha);
									 
	if (push.outlineEnabled > 0.0) {
		float w = 1.0 - push.outlineWidth;
		alpha = smoothstep(w - smoothWidth, w + smoothWidth, distance);
        rgb += mix(vec3(alpha), inColor.rgb, alpha);
    }									 
									 
    outFragColor = vec4(rgb, alpha);
}