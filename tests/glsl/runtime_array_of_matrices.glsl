#version 450

layout(std430, set = 0, binding = 0) readonly buffer Matrices {
    mat4 mats[];
};

layout(set = 0, binding = 1) buffer Out {
    vec4 result;
};

void main() {
    result = mats[3][2];
}
