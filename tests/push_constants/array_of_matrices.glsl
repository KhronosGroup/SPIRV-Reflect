#version 450

layout(push_constant) uniform PushConstants {
    mat4 single;
    mat4 columnMajor[2];
    layout(row_major) mat4 rowMajor[2];
    mat3x2 narrow[3];
} pc;

layout(set = 0, binding = 0) buffer Out {
    vec4 result;
};

void main() {
    result = pc.single[1] + pc.columnMajor[1][2] + pc.rowMajor[0][3] + vec4(pc.narrow[2][1], 0.0, 0.0);
}
