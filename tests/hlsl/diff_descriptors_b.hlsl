/*
Build with DXC using -O0 to preserve unused types:

   dxc -spirv -O0 -T ps_5_0 -Fo diff_descriptors_b.spv diff_descriptors_b.hlsl

*/

cbuffer MyCBuffer : register(b0, space0) {
  float x;
};

struct Data { float x; };
ConstantBuffer<Data>            MyConstantBuffer : register(b1, space0);

Texture1D                       MyTexture1D : register(t0, space2);
Texture2D                       MyTexture2D : register(t1, space2);
Texture3D                       MyTexture3D : register(t2, space2);

Texture1DArray                  MyTexture1DArray : register(t4, space1);
Texture2DArray                  MyTexture2DArray : register(t5, space1);

RWTexture1D<float4>             MyRWTexture1D : register(u1, space1);
RWTexture2D<float4>             MyRWTexture2D : register(u3, space1);
RWTexture3D<float4>             MyRWTexture3D : register(u6, space1);

RWTexture1DArray<float4>        MyRWTexture1DArray : register(u7, space3);
RWTexture2DArray<float4>        MyRWTexture2DArray : register(u8, space3);

Texture2DMS<float4>             MyTexture2DMS : register(t15, space4);
Texture2DMSArray<float4>        MyTexture2DMSArray : register(t18, space4);

TextureCube<float4>             MyTextureCube : register(t19, space4);
TextureCubeArray<float4>        MyTextureCubeArray : register(t22, space4);

tbuffer MyTBuffer : register(t3, space5) {
  float q;
};

struct Data2 {
  float4 x;
};

TextureBuffer<Data2>            MyTextureBuffer : register(t17, space5);

Buffer                          MyBuffer : register(t1, space6);
RWBuffer<float4>                MyRWBuffer : register(u4, space6);

StructuredBuffer<float>         MyStructuredBuffer : register(t18, space17);
RWStructuredBuffer<float>       MyRWStructuredBuffer : register(u31, space17);

AppendStructuredBuffer<float>   MyAppendStructuredBuffer : register(u3, space8);
ConsumeStructuredBuffer<float>  MyConsumeStructuredBuffer : register(u7, space8);

ByteAddressBuffer               MyByteAddressBuffer : register(t1, space11);
RWByteAddressBuffer             MyRWByteAddressBuffer : register(u6, space11);

float4 main(float P : SV_POSITION) : SV_TARGET
{
  return float4(0, 0, 0, 0);
}
