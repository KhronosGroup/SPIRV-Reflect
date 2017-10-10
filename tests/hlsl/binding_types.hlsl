
Texture2D             MyTexture     : register(t0);

SamplerState          MySampler     : register(s1);

RWTexture2D<float4>   MyRWTexture2D : register(u2);

cbuffer               MyCBuffer     : register(b3) {
  float4 color;
};

float4 main(float4 sv_pos : SV_POSITION) : SV_TARGET {
  float4 ret = MyTexture.Sample(MySampler, float2(0, 0)) + color;
  MyRWTexture2D[int2(0, 0)] = ret;
  return ret;
}