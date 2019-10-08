//
//  basic.metal
//  tutorial_03
//
//  Created by TAKEZOH on 2019/10/04.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

#if 1
// 頂点定義.
typedef struct
{
	float3 position [[attribute(0)]];
	float2 texCoord [[attribute(1)]];
} Vertex;

// フラグメントシェーダーに渡す値.
typedef struct
{
	float4 position [[position]];
	float2 texCoord;
} ColorInOut;

// 頂点シェーダー.
vertex ColorInOut vertFunc(Vertex in [[stage_in]])
{
	ColorInOut out;
	
	out.position = float4(in.position, 1.0);
	out.texCoord = in.texCoord;
	
	return out;
}

// フラグメントシェーダー.
fragment float4 fragFunc(ColorInOut in [[stage_in]], texture2d<half> colorMap [[texture(0)]])
{
	constexpr sampler colorSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);
	//constexpr sampler colorSampler(mip_filter::none, mag_filter::nearest, min_filter::nearest);
	half4 colorSample = colorMap.sample(colorSampler, in.texCoord.xy);
	
	return float4(colorSample);
}
#else
// 頂点シェーダー.
vertex float4 vertFunc(const device packed_float3* vertexArray [[buffer(0)]], unsigned int vID [[vertex_id]])
{
	return float4(vertexArray[vID], 1.0);
}

// フラグメントシェーダー.
fragment half4 fragFunc()
{
	// 記述ミスがあるとビルド時にコンパイルエラーになる.
	//return falf4(1.0, 0.0, 0.0, 1.0);
	return half4(1.0, 0.0, 0.0, 1.0);
}
#endif

