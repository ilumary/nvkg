#version 460

layout (binding = 1) uniform sampler2DArray samplerArray;

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec3 in_uv;
layout (location = 3) in vec3 in_view_vec;
layout (location = 4) in vec3 in_light_vec;

layout (location = 0) out vec4 outFragColor;

void main() {
    //vec4 color = texture(samplerArray, in_uv) * vec4(in_color, 1.0);	
	vec4 color = vec4(in_color, 1.0);
	vec3 N = normalize(in_normal);
	vec3 L = normalize(in_light_vec);
	vec3 V = normalize(in_view_vec);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, L), 0.1) * in_color;
	vec3 specular = (dot(N,L) > 0.0) ? pow(max(dot(R, V), 0.0), 16.0) * vec3(0.75) * color.r : vec3(0.0);
	outFragColor = vec4(diffuse * color.rgb + specular, 1.0);
	//outFragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
