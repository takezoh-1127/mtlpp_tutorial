//
//  phong.metal
//  tutorial_08
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
	float2 texcoord [[attribute(2)]];
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
	float3 L;
	float3 N;
	float3 E;
	float specularPower;
}VertexOut;


// ローカルスペースでの計算に置き換える.
vertex VertexOut phongVS(VertexIn in [[stage_in]], constant UniformVS& inUniform[[buffer(3)]])
{
	VertexOut out;
	
	// ワールド座標を求める.
	float4 position = float4(in.position, 1.0);
	
	out.position = inUniform.WVP * position;
	
	out.texcoord = float2(in.texcoord);
	
	//
	float3 N = in.normal;
	// ローカルスペースに変換された光源の向き.
	float3 L = normalize(-inUniform.lightDir);
	
	// saturate() 0.0 - 1.0 にクランプ.
	float d = saturate(dot(N, L));
	
	float3 ambient = inUniform.ambientColor;
	float3 diffuse = inUniform.diffuseColor * d;
		
	float3 color = diffuse + ambient;
	color = clamp(color, 0.0, 1.0);
	
	out.color = float4(color,1.0);
	
	out.L = L;
	out.N = N;
	out.E = normalize(inUniform.cameraPos - in.position);
	out.specularPower = inUniform.specularPower;
	
	return out;
}

fragment float4 phongFS(VertexOut in[[stage_in]], texture2d<float> tex [[texture(0)]])
{
	constexpr sampler quadSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);
	
	float3 L = in.L;
	//float3 E = normalize(in.E);
	float3 E = in.E;
	float3 N = in.N;
	
	float3 H = normalize(L + E);	// ハーフベクトル.
	
	float4 out = (tex.sample(quadSampler, in.texcoord) * in.color) + pow(max(0.0,dot(N,H)), in.specularPower);
	
	return out;
	//return float4(1.0, 0.0, 0.0, 1.0);
}

