#version 450
#extension GL_EXT_scalar_block_layout : enable

// 16 byte large storage buffer
// (Note: In HLSL/Slang all storage buffers are runtime array so there is no size)
layout(set = 0, binding = 0, scalar) buffer D0 {
    int a;
    int b[3];
} ssbo;

// runtime storage buffer - no known length
layout(set = 2, binding = 0, scalar) buffer D1 {
    int x;
    int y[];
} ssbo_runtime;

// 16 byte large uniform buffer
layout(set = 1, binding = 0, scalar) uniform D2 {
    int s;
    int t[3];
} ubo;

void main() {
    ssbo.b[1] = ubo.t[1] + ssbo_runtime.y[ubo.s];
}
