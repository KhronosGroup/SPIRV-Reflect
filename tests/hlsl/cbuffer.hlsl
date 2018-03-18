

struct Under16 {
  float x;
  float y;
};

struct Over16 {
  float3x3 m3;
  float4x4 m4;
};

struct Nested3 {
  float2 x2;
  float2 y2;
};

struct Nested2 {
  float   x;
  Nested3 nested3;
};

struct Nested1 {
  float   x;
  Nested2 nested2;
};

cbuffer MyCBuffer {
  float   x;
  float   y;
  Under16 under_16;
  Over16  over_16;
  Nested1 nested1;
  float   z;
};

float4 main(float4 pos : POSITION) : SV_Position
{
  return float4(x, 0, 0, 0);
}