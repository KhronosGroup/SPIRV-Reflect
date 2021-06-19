#version 450
#pragma shader_stage(compute)

layout(set = 0, binding = 0) buffer Matrices {
	mat4 in_matrices[4];
	mat4 out_matrices[];
};

void main() {
	uint index = gl_GlobalInvocationID.x;
	out_matrices[index] = in_matrices[index % 4];
}
