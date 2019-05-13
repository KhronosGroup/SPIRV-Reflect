
cbuffer MyParams : register(b0)
{
	float4x4 MvpMatrix;
	float4x4 NotUsedNormalMatrix;
	float3   Offset;
	float    NotUsed1;
	float2   NotUsed2;
	float3   NotUsed3;
	float3   MoreOffset;
	float2   NotUsed4;
	float3   NotUsed5;
	float3   LastOffset;
	float3   NotUsed6;
}

struct NestedUsedParams {
	float3 Offset;
};

struct NestedUnusedParams {
	float  NotUsed1;
	float2 NotUsed2;
	float3 NotUsed3;
};

struct UsedParams {
	float3             Position;
	float3             Color;
	float3             Normal;
	NestedUnusedParams NotUsedNested;
	NestedUsedParams   UsedNested;
	float              NotUsed1;
};

struct UnusedParams {
	float              NotUsed1;
	float2             NotUsed2;
	float3             NotUsed3;
	NestedUnusedParams NotUsedNested;
};

struct Params2 {
	float4       PostTransformOffset;
	float        Scale;
	float3       Mask;
	UsedParams   Used;
	UnusedParams Unused;
};

ConstantBuffer<Params2> MyParams2 : register(b1);

float4 main(float3 Position : Position) : SV_POSITION
{
	float4 result = mul(MvpMatrix, float4(Position + Offset, 1)) + MyParams2.PostTransformOffset;
	result.xyz *= MyParams2.Mask;
	result.xyz *= MyParams2.Used.Position;
	result.xyz += MyParams2.Used.Normal;
	result.xyz += MoreOffset;
	result.xyz += LastOffset;
	result.xyz += MyParams2.Used.UsedNested.Offset;
	return result;
}