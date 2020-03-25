//
//  mosaic.metal
//  MosaicFilter
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




vertex VertexOut mosaicVS(VertexIn in [[stage_in]])
{
	VertexOut out;

	out.position = float4(in.position, 1.0);
	out.texcoord = float2(in.texcoord);
	
	return out;
}

fragment float4 mosaicFS(VertexOut in[[stage_in]], texture2d<float> tex [[texture(0)]])
{
	constexpr sampler quadSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);
	
	//float grids = mix(30.0f, 640.0f, inUniform.t);
	const float grids = 30.0f;
	
	float2 tex_coord = floor(grids * in.texcoord + 0.5f) / grids;
	
	//float4 out = float4(0.0, 0.0, 1.0, 1.0);
	float3 color = tex.sample(quadSampler, tex_coord).xyz;
	
	float4 out;
	
	out.xyz = color;
	//out.xyz = mix(color, mosaic, inUniform.t);
	out.w = 1.0;
	
	return out;
	//return float4(1.0, 0.0, 0.0, 1.0);
}

