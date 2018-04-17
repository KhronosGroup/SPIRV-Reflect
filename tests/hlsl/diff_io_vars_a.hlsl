
struct VSInput {
  [[vk::location(0)]] float4  Position  : POSITION;
  [[vk::location(1)]] float3  Normal    : NORMAL;
  [[vk::location(2)]] float3  Color     : COLOR_00001;
  [[vk::location(3)]] float   Alpha     : OPACITY_512;
  [[vk::location(4)]] float4  Scaling   : SCALE_987654321;
  [[vk::location(5)]] float2  TexCoord0 : TEXCOORD0;
  [[vk::location(6)]] float2  TexCoord1 : TEXCOORD1;
  [[vk::location(7)]] float2  TexCoord2 : TEXCOORD2;
};

struct VSOutput {
  [[vk::location(0)]] float4  Position  : SV_POSITION;
  [[vk::location(1)]] float3  Normal    : NORMAL;
  [[vk::location(2)]] float3  Color     : COLOR_00001;
  [[vk::location(3)]] float   Alpha     : OPACITY_512;
  [[vk::location(4)]] float4  Scaling   : SCALE_987654321;
  [[vk::location(5)]] float2  TexCoord0 : TEXCOORD0;
  [[vk::location(6)]] float2  TexCoord1 : TEXCOORD1;
  [[vk::location(7)]] float2  TexCoord2 : TEXCOORD2;
  [[vk::location(8)]] uint    VertexID  : VERTEX_ID;
};

VSOutput main(VSInput input, uint VertexID : SV_VertexID)
{
  VSOutput ret;
  ret.Position  = input.Position;
  ret.Normal    = input.Normal;
  ret.Color     = input.Color;
  ret.Alpha     = input.Alpha;
  ret.Scaling   = input.Scaling;
  ret.TexCoord0 = input.TexCoord0;
  ret.TexCoord1 = input.TexCoord1;
  ret.TexCoord2 = input.TexCoord2;
  ret.VertexID  = VertexID;
  return ret;
}