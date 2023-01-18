#version 460

// Vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

// Instanced attributes
layout (location = 4) in vec3 instance_pos;
layout (location = 5) in vec3 instance_scale;
layout (location = 6) in vec3 instance_rot;
//layout (location = 7) in float instance_tex_index;

layout (binding = 0) uniform GlobalData {
	mat4 projection;
	mat4 modelview;
	vec4 lightPos;
} globalData;

layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec3 out_color;
layout (location = 2) out vec3 out_uv;
layout (location = 3) out vec3 out_view_vec;
layout (location = 4) out vec3 out_light_vec;

void main() {
    out_color = color;
	//out_uv = vec3(uv, instance_tex_index);
	out_uv = vec3(uv, 0);

	vec4 locPos = vec4(position.xyz, 1.0);
	vec4 pos = vec4((locPos.xyz * instance_scale) + instance_pos, 1.0);

	gl_Position = globalData.projection * globalData.modelview * pos;
	out_normal = mat3(globalData.modelview) * normal;

	pos = globalData.modelview * vec4(position.xyz + instance_pos, 1.0);
	vec3 lPos = mat3(globalData.modelview) * globalData.lightPos.xyz;
	out_light_vec = lPos - pos.xyz;
	out_view_vec = -pos.xyz;
}