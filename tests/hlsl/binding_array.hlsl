
SamplerState          MySampler[4][3][2][1] : register(s0);

Texture2D             MyTexture[2] : register(t1);

float4 main(float4 sv_pos : SV_POSITION) : SV_TARGET {
  float4 ret = MyTexture[0].Sample(MySampler[0][0][0][0], float2(0, 0));
  return ret;
}