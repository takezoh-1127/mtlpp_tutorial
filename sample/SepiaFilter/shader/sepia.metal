//
//  sepia.metal
//  SSepiaFilter
//
//  Created by TAKEZOH on 2020/03/23.
//  Copyright © 2020 TAKEZOH. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;


// 頂点シェーダーに渡す頂点バッファ.
typedef struct
{
	float3 position [[attribute(0)]];
	//float3 normal [[attribute(1)]];
	float2 texcoord [[attribute(1)]];
}VertexIn;

// フラグメントシェーダーに渡す頂点出力.
typedef struct
{
	float4 position [[position]];
	float2 texcoord;
}VertexOut;

// フラグメントシェーダーに渡すユニフォームバッファ.
typedef struct
{
	float t;
}UniformFS;

// セピアフィルター.
vertex VertexOut sepiaVS(VertexIn in [[stage_in]])
{
	VertexOut out;

	out.position = float4(in.position, 1.0);
	out.texcoord = float2(in.texcoord);
	
	return out;
}

fragment float4 sepiaFS(VertexOut in[[stage_in]], constant UniformFS& inUniform [[buffer(0)]], texture2d<float> tex [[texture(0)]])
{
	constexpr sampler quadSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);
	
	const float3 RGB2Y = { 0.29900, 0.58700, 0.11400 };
	
#if 1
	const float3 Cb2RGB = { 0.0f, -0.34414f, 1.772f };
	const float3 Cr2RGB = { 1.402f, -0.71414f, 0.0f };
#else
#if 0
	float3x3 YCbCr2RGB =
	{
		{ 1.0, 0.0, 1.40200 },
		{ 1.0, -0.34414, -0.71414 },
		{ 1.0, 1.77200, 0.0 },
	};
	
	// 行優先？.
	YCbCr2RGB = transpose(YCbCr2RGB);
#else
	const float3x3 YCbCr2RGB =
	{
		{ 1.0, 1.0, 1.0 },
		{ 0.0, -0.34414, 1.77200 },
		{ 1.40200, -0.71414, 0.0 },
	};
#endif
#endif
	
	//float4 out = float4(0.0, 0.0, 1.0, 1.0);
	float3 color = tex.sample(quadSampler, in.texcoord).xyz;
	
	float3 YCbCr;
	
	YCbCr.x = dot(color, RGB2Y);	// Y
	YCbCr.y = -0.2;					// Cb
	YCbCr.z = 0.1;					// Cr
	
	//float3 sepia = YCbCr2RGB * YCbCr;
	float3 sepia = YCbCr.x + Cb2RGB * YCbCr.y + Cr2RGB * YCbCr.z;
	float4 out;
	
	out.xyz = mix(color, sepia, inUniform.t);
	//out.xyz = YCbCr2RGB * YCbCr;
	out.w = 1.0;
	
	return out;
	//return float4(1.0, 0.0, 0.0, 1.0);
}
