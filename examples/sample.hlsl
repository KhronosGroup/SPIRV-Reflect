Texture2D     MyTexture : register(t0, space0);
SamplerState  MySampler : register(s1, space1);

float4 main(float4 sv_pos : SV_POSITION) : SV_TARGET
{
  return sv_pos;
}