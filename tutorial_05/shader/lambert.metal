//
//  lambert.metal
//  tutorial_05
//
//  Created by TAKEZOH on 2019/11/05.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

//
typedef struct
{
	float3 position [[attribute(0)]];
	float3 normal [[attribute(1)]];
	float2 texcoord [[attribute(2)]];
}VertexIn;

// 頂点シェーダーに渡すユニフォームバッファ.
typedef struct
{
	float4x4 viewProjection;
	float4x4 worldTransform;
}UniformVS;

// フラグメントシェーダーに渡すユニフォームバッファ.
typedef struct
{
	packed_float3 lightDir;
	packed_float3 diffuseColor;
	packed_float3 ambientColor;
}UniformFS;

// フラグメントシェーダーに渡す頂点出力.
typedef struct
{
	float4 position [[position]];
	float2 texcoord;
	float3 worldPosition;
	float3 worldNormal;
}VertexOut;


/**
 */
vertex VertexOut lambertVS(VertexIn in [[stage_in]], constant UniformVS& inUniform [[buffer(3)]])
{
	VertexOut out;
	
	float4 position = float4(in.position, 1.0);
	out.position = inUniform.viewProjection * inUniform.worldTransform * position;
	//out.texcoord = float2(in.texcoord[0], in.texcoord[1]);
	out.texcoord = float2(in.texcoord);
	
	// transform position to world space.
	out.worldPosition = float3(inUniform.worldTransform * position);
	
	// transform normal into world space. (w = 0.0)
	out.worldNormal = float3(inUniform.worldTransform * float4(in.normal, 0.0));
	
	return out;
}


/**
 */
fragment float4 lambertFS(VertexOut in[[stage_in]], constant UniformFS& inUniform[[buffer(3)]], texture2d<float> tex [[texture(0)]])
{
	constexpr sampler quadSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);
	
	// surface normal.
	float3 N = normalize(in.worldNormal);
	// vector form surface to light.
	float3 L = normalize(-inUniform.lightDir);
	//
	//float3 V = normalize(inUniform.cameraPos - in.worldPosition);
	//
	//float3 R = normalize(reflect(-L, N));
	
	float d = dot(N, L);
	
	float3 color = inUniform.ambientColor;
	
	//if(d > 0.0)
	{
		float3 diffuse = inUniform.diffuseColor * max(0.0, d);
		
		color += diffuse;
		
		color = clamp(color, 0.0, 1.0);
	}	
	
    float4 out = tex.sample(quadSampler, in.texcoord) * float4(color, 1.0);
	
	return out;
	//return float4(1.0, 0.0, 0.0, 1.0);
}
