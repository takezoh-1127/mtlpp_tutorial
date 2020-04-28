//
//  normalmap.metal
//  NormalMapping
//
//  Created by TAKEZOH on 2019/12/20.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;


// 頂点シェーダーに渡す頂点バッファ.
typedef struct
{
	float3 position [[attribute(0)]];
	float3 normal [[attribute(1)]];
	float3 tangent [[attribute(2)]];
	float3 binormal [[attribute(3)]];
	float2 texcoord [[attribute(4)]];
}VertexIn;

// 頂点シェーダーに渡すユニフォームバッファ.
typedef struct
{
	float4x4 VP;			// view -> prjection
	float4x4 LW;			// local -> world
	float4x4 WVP;			// local -> world -> view -> projection
	float4x4 invLW;			// invert LW
	packed_float3 lightDir;
	packed_float3 cameraPos;
	packed_float3 diffuseColor;
	packed_float3 specularColor;
	packed_float3 ambientColor;
	float specularPower;
}UniformVS;

// フラグメントシェーダーに渡す頂点出力.
typedef struct
{
	float4 position [[position]];
	float2 texcoord;
	float4 color;
	float4 ambient;
	float3 L;
	float3 N;
	float3 E;
	float3 specularColor;
	float specularPower;
}VertexOut;


// 
vertex VertexOut normalMappingVS(VertexIn in [[stage_in]], constant UniformVS& inUniform[[buffer(5)]])
{
	VertexOut out;
	
	float4 position = float4(in.position, 1.0);
	
	// スクリーン座標を求める.
	out.position = inUniform.WVP * position;
	
	out.texcoord = float2(in.texcoord);
	
	float3 N = in.normal;
	float3 T = in.tangent;
	float3 B = in.binormal;
	//float3 B = cross(N, T);
	
	out.color = float4(inUniform.diffuseColor, 1.0);
	out.ambient = float4(inUniform.ambientColor, 1.0);
	
	// ローカルスペースに変換された光源の向き.
	float3 L = -normalize(inUniform.lightDir);
	//float3 L = normalize(inUniform.lightDir);
	
	out.L.x = dot(L, T);
	out.L.y = dot(L, B);
	out.L.z = dot(L, N);
	
	//out.N = N;
	
	float3 E = inUniform.cameraPos - in.position;
	
	out.E.x = dot(E, T);
	out.E.y = dot(E, B);
	out.E.z = dot(E, N);
	
	out.specularColor = inUniform.specularColor;
	out.specularPower = inUniform.specularPower;
	
	return out;
}

fragment float4 normalMappingFS(VertexOut in[[stage_in]], texture2d<float> tex [[texture(0)]], texture2d<float> normalMap [[texture(1)]])
{
	//return float4(1.0, 0.0, 0.0, 1.0);
	
	constexpr sampler quadSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);
	
	//float3 normal = normalMap.sample(quadSampler, in.texcoord).xyz;
	
	//return float4(normal, 1.0);
	
	// 0.0〜1.0 を -1.0〜1.0 に変換.
	float3 normal = 2.0 * normalMap.sample(quadSampler, in.texcoord).xyz - 1.0;
	//normal.xy = normal.xy * 2.0 - 1.0;
	//normal = normal * 2.0 - 1.0;
	
	normal = normalize(normal);
	
	//return float4(normal, 1.0);
	
	float3 L = normalize(in.L);
	float3 E = normalize(in.E);
	float3 N = normal;
	
	//return float4(N, 1.0);
	
	// saturate() 0.0 - 1.0 にクランプ.
	float d = saturate(dot(N, L));
	
	float3 H = normalize(L + E);	// ハーフベクトル.
	
	float reflection = max(0.0, dot(N, H));
	float specular = pow(reflection, in.specularPower);
	float4 specularColor = float4(in.specularColor, 1.0) * specular;
	
	float4 out = (tex.sample(quadSampler, in.texcoord) * ((in.color * d) + in.ambient)) + specularColor;
	//float4 out = (normalMap.sample(quadSampler, in.texcoord) * in.color) + pow(max(0.0,dot(N,H)), in.specularPower);
	
	return out;
	//return float4(1.0, 0.0, 0.0, 1.0);
}

