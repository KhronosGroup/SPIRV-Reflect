
struct VSInput {
  [[vk::location( 0)]] float4  Position  : POSITION;
  [[vk::location( 1)]] float4  Normal    : NORMAL;
  [[vk::location( 4)]] float2  Color     : COLOR_00001;
  [[vk::location( 5)]] float   Alpha     : OPACITY_512;
  [[vk::location( 7)]] float4  Scaling   : SCALE_987654321;
  [[vk::location(10)]] float2  TexCoord0 : TEXCOORD0;
  [[vk::location(13)]] float2  TexCoord1 : TEXCOORD1;
  [[vk::location(15)]] float2  TexCoord2 : TEXCOORD2;
};

struct VSOutput {
  [[vk::location( 1)]] float4  Position   : SV_POSITION;
  [[vk::location( 2)]] float4  Normal     : NORMAL;
  [[vk::location( 3)]] float2  Color      : COLOR_00001;
  [[vk::location( 7)]] float   Alpha      : OPACITY_512;
  [[vk::location( 9)]] float4  Scaling    : SCALE_987654321;
  [[vk::location(10)]] float2  TexCoord0  : TEXCOORD0;
  [[vk::location(23)]] float2  TexCoord1  : TEXCOORD1;
  [[vk::location(55)]] float2  TexCoord2  : TEXCOORD2;
  [[vk::location(62)]] uint    InstanceID : VERTEX_ID;
};

VSOutput main(VSInput input, uint InstanceID : SV_InstanceID)
{
  VSOutput ret;
  ret.Position   = input.Position;
  ret.Normal     = input.Normal;
  ret.Color      = input.Color;
  ret.Alpha      = input.Alpha;
  ret.Scaling    = input.Scaling;
  ret.TexCoord0  = input.TexCoord0;
  ret.TexCoord1  = input.TexCoord1;
  ret.TexCoord2  = input.TexCoord2;
  ret.InstanceID = InstanceID;
  return ret;
}