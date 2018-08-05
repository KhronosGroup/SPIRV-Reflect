
SamplerState          MySampler[4][3][2][1] : register(s0);
Texture2D             MyTexture[2] : register(t1);

float4 main(float4 sv_pos : SV_POSITION) : SV_TARGET {
  float4 ret0 = MyTexture[0].Sample(MySampler[1][0][0][0], float2(0, 0));
  float4 ret1 = MyTexture[0].Sample(MySampler[1][2][1][0], float2(0, 0));
  return ret0 + ret1;
}