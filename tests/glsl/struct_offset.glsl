#version 450

layout(location=0) in vec2 aPos;
layout(location=1) in vec2 aUV;

struct S
{
    vec3 a;
    vec2 b;
};

layout(push_constant) uniform uPushConstant { layout(offset = 8) vec2 uScale; float d2; layout(offset = 4) float d1;} vpc1;

layout(set=0, binding=0, std140) uniform uBuffer { layout(offset = 4) float d0; layout(offset = 16) vec2 uTranslate; layout(offset = 8) float d1;} vpc2;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec2 Pos; vec2 UV; } Out;

void main(void)
{
    Out.UV = aUV;
    Out.Pos = aPos;
    gl_Position = vec4(aPos * vpc1.uScale + vpc2.uTranslate, 0, 1);
}
