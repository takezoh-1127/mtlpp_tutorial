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
	float3 cameraPos;
	float3 worldPos;
	float3 fragPos;
	float3 tangentLightPos;
	float3 tangentViewPos;
	float3 tangentFragPos;
	float4 color;
	float3 L;
	float3 N;
	float3 E;
	float3 specularColor;
	float specularPower;
	float3 normal;
	float3 tangent;
	float3 binormal;
}VertexOut;


// 
vertex VertexOut normalMappingVS(VertexIn in [[stage_in]], constant UniformVS& inUniform[[buffer(5)]])
{
	VertexOut out;
	
	// ワールド座標を求める.
	float4 position = float4(in.position, 1.0);
	
	out.position = inUniform.WVP * position;
	
	out.texcoord = float2(in.texcoord);
	
	//---
	//float3x3 normalMatrix = transpose(float3x3(inUniform.invLW[0].xyz,inUniform.invLW[1].xyz,inUniform.invLW[2].xyz));
	float3x3 normalMatrix = float3x3(inUniform.LW[0].xyz,inUniform.LW[1].xyz,inUniform.LW[2].xyz);
	//float3x3 normalMatrix = transpose(float3x3(inUniform.LW[0].xyz,inUniform.LW[1].xyz,inUniform.LW[2].xyz));
	
	out.normal = normalize(normalMatrix * in.normal);
	out.tangent = normalize(normalMatrix * in.tangent);
	out.binormal = normalize(normalMatrix * in.binormal);
	
	//
	//float3 N = normalMatrix * in.normal;
	float3 N = in.normal;
	// ローカルスペースに変換された光源の向き.
	// ワールドスペース.
	float3 L = normalize(-inUniform.lightDir);
	//float3 L = normalize(inUniform.lightDir);
	
	// saturate() 0.0 - 1.0 にクランプ.
	//float d = saturate(dot(N, L));
	
	float3 ambient = inUniform.ambientColor;
	//float3 diffuse = inUniform.diffuseColor * d;
	float3 diffuse = inUniform.diffuseColor;
		
	float3 color = diffuse + ambient;
	color = clamp(color, 0.0, 1.0);
	
	out.color = float4(color,1.0);
	
	out.L = L;
	out.N = N;
	out.E = normalize(inUniform.cameraPos - in.position);
	//out.E = normalize(inUniform.cameraPos - (float3)(inUniform.LW * position));
	
	out.specularColor = inUniform.specularColor;
	out.specularPower = inUniform.specularPower;
	
	out.cameraPos = inUniform.cameraPos;
	out.worldPos = (float3)(inUniform.LW * position);
	
	return out;
}

fragment float4 normalMappingFS(VertexOut in[[stage_in]], texture2d<float> tex [[texture(0)]], texture2d<float> normalMap [[texture(1)]])
{
	//return float4(1.0, 0.0, 0.0, 1.0);
	
	constexpr sampler quadSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);
	
	float3 normal = normalMap.sample(quadSampler, in.texcoord).xyz;
	
	//normal.xy = normal.xy * 2.0 - 1.0;
	//normal = normal * 2.0 - 1.0;
	
	normal = normalize(normal);
	
	//return float4(normal, 1.0);
	
	float3x3 tangentMatrix = float3x3(in.tangent, in.binormal, in.normal);
	
	float3 L = in.L;
	//float3 E = normalize(in.E);
	//float3 E = in.E;
	//float3 N = in.N;
	float3 E = in.cameraPos - in.worldPos;
	float3 N = (float3)(tangentMatrix * normal);
	//float3 N = in.N;
	
	float3 H = normalize(L + E);	// ハーフベクトル.
	
	float reflection = max(0.0,dot(N,H));
	float specular = pow(reflection, in.specularPower);
	float4 specularColor = float4(in.specularColor, 1.0) * specular;
	float4 out = (tex.sample(quadSampler, in.texcoord) * in.color) + specularColor;
	//float4 out = (normalMap.sample(quadSampler, in.texcoord) * in.color) + pow(max(0.0,dot(N,H)), in.specularPower);
	
	return out;
	//return float4(1.0, 0.0, 0.0, 1.0);
}

