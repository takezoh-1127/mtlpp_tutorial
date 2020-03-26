//
//  mono.metal
//  MonoFilter
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

// モノクロフィルター.
vertex VertexOut monoVS(VertexIn in [[stage_in]])
{
	VertexOut out;

	out.position = float4(in.position, 1.0);
	out.texcoord = float2(in.texcoord);
	
	return out;
}

fragment float4 monoFS(VertexOut in[[stage_in]], constant UniformFS& inUniform [[buffer(0)]], texture2d<float> tex [[texture(0)]])
{
	constexpr sampler quadSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);
	
	const float3 RGB2Y = { 0.29900, 0.58700, 0.11400 };
	
	//float4 out = float4(0.0, 0.0, 1.0, 1.0);
	float3 color = tex.sample(quadSampler, in.texcoord).xyz;
	
	float4 out;
	
	float3 mono = dot(color, RGB2Y);
	
	out.xyz = mix(color, mono, inUniform.t);
	//out.xyz = mix(color, mono, 1.0);
	out.w = 1.0;
	
	return out;
}

