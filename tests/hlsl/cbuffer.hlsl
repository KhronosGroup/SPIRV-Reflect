

struct MaterialData {
  float3  Color;
  float   Specular;
  float   Diffuse;
};

cbuffer MyCBuffer {
  float4x4      ModelMatrix;
  float4x4      ProjectionMatrix;
  float         Time;
  float3        Scale;  
  float2        UvOffset;
  MaterialData  Material;
  bool          EnableTarget;
};

float4 main(float4 pos : POSITION) : SV_Position
{
  return float4(Time, 0, 0, 0);
}