//
//  gaussian.metal
//  GaussianFilter
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
	float2 tex0;
	float2 tex1;
	float2 tex2;
	float2 tex3;
	float2 tex4;
	float2 tex5;
	float2 tex6;
	float2 tex7;
}VertexOut;

// フラグメントシェーダーに渡すユニフォームバッファ.
typedef struct
{
	float t;
	float w[8];
}UniformFS;

// 横方向のぼかし.
vertex VertexOut gaussianVS_pass1(VertexIn in [[stage_in]])
{
	VertexOut out;

	out.position = float4(in.position, 1.0);
	out.texcoord = float2(in.texcoord);

	constexpr float MAP_WIDTH = 640.0f;
	
	out.tex0 = in.texcoord + float2(-1.0 / MAP_WIDTH, 0.0);
	out.tex1 = in.texcoord + float2(-3.0 / MAP_WIDTH, 0.0);
	out.tex2 = in.texcoord + float2(-5.0 / MAP_WIDTH, 0.0);
	out.tex3 = in.texcoord + float2(-7.0 / MAP_WIDTH, 0.0);
	out.tex4 = in.texcoord + float2(-9.0 / MAP_WIDTH, 0.0);
	out.tex5 = in.texcoord + float2(-11.0 / MAP_WIDTH, 0.0);
	out.tex6 = in.texcoord + float2(-13.0 / MAP_WIDTH, 0.0);
	out.tex7 = in.texcoord + float2(-15.0 / MAP_WIDTH, 0.0);
	
	return out;
}

fragment float4 gaussianFS_pass1(VertexOut in[[stage_in]], constant UniformFS& inUniform [[buffer(0)]], texture2d<float> tex [[texture(0)]])
{
	constexpr sampler quadSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);
	
	float2 offsetX = float2(16.0 / 640.0f, 0.0);
	//float4 out = float4(1.0, 0.0, 0.0, 1.0);
	//return tex.sample(quadSampler, in.texcoord);
	
	float4 out;
	
	out = inUniform.w[0] * (tex.sample(quadSampler, in.tex0) + tex.sample(quadSampler, in.tex7 + offsetX));
	out += inUniform.w[1] * (tex.sample(quadSampler, in.tex1) + tex.sample(quadSampler, in.tex6 + offsetX));
	out += inUniform.w[2] * (tex.sample(quadSampler, in.tex2) + tex.sample(quadSampler, in.tex5 + offsetX));
	out += inUniform.w[3] * (tex.sample(quadSampler, in.tex3) + tex.sample(quadSampler, in.tex4 + offsetX));
	out += inUniform.w[4] * (tex.sample(quadSampler, in.tex4) + tex.sample(quadSampler, in.tex3 + offsetX));
	out += inUniform.w[5] * (tex.sample(quadSampler, in.tex5) + tex.sample(quadSampler, in.tex2 + offsetX));
	out += inUniform.w[6] * (tex.sample(quadSampler, in.tex6) + tex.sample(quadSampler, in.tex1 + offsetX));
	out += inUniform.w[7] * (tex.sample(quadSampler, in.tex7) + tex.sample(quadSampler, in.tex0 + offsetX));
	
	//out.xyz = out.xyz * 0.5;
	
	out.xyz = mix(tex.sample(quadSampler, in.texcoord).xyz, out.xyz, inUniform.t);
	
	out.w = 1.0;
	
	return out;
	//return float4(1.0, 0.0, 0.0, 1.0);
}


// 縦方向のぼかし.
vertex VertexOut gaussianVS_pass2(VertexIn in [[stage_in]])
{
	VertexOut out;

	out.position = float4(in.position, 1.0);
	out.texcoord = float2(in.texcoord);
	
	constexpr float MAP_WIDTH = 480.0f;
	
	out.tex0 = in.texcoord + float2(0.0, -1.0 / MAP_WIDTH);
	out.tex1 = in.texcoord + float2(0.0, -3.0 / MAP_WIDTH);
	out.tex2 = in.texcoord + float2(0.0, -5.0 / MAP_WIDTH);
	out.tex3 = in.texcoord + float2(0.0, -7.0 / MAP_WIDTH);
	out.tex4 = in.texcoord + float2(0.0, -9.0 / MAP_WIDTH);
	out.tex5 = in.texcoord + float2(0.0, -11.0 / MAP_WIDTH);
	out.tex6 = in.texcoord + float2(0.0, -13.0 / MAP_WIDTH);
	out.tex7 = in.texcoord + float2(0.0, -15.0 / MAP_WIDTH);
	
	return out;
}

fragment float4 gaussianFS_pass2(VertexOut in[[stage_in]], constant UniformFS& inUniform [[buffer(0)]], texture2d<float> tex [[texture(0)]])
{
	constexpr sampler quadSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);
	
	//float4 out = float4(0.0, 0.0, 1.0, 1.0);
	//float4 out = tex.sample(quadSampler, in.texcoord);
	//return tex.sample(quadSampler, in.texcoord);
	
	float2 offsetX = float2(0.0, 16.0 / 480.0f);
	
	float4 out;
	
	out = inUniform.w[0] * (tex.sample(quadSampler, in.tex0) + tex.sample(quadSampler, in.tex7 + offsetX));
	out += inUniform.w[1] * (tex.sample(quadSampler, in.tex1) + tex.sample(quadSampler, in.tex6 + offsetX));
	out += inUniform.w[2] * (tex.sample(quadSampler, in.tex2) + tex.sample(quadSampler, in.tex5 + offsetX));
	out += inUniform.w[3] * (tex.sample(quadSampler, in.tex3) + tex.sample(quadSampler, in.tex4 + offsetX));
	out += inUniform.w[4] * (tex.sample(quadSampler, in.tex4) + tex.sample(quadSampler, in.tex3 + offsetX));
	out += inUniform.w[5] * (tex.sample(quadSampler, in.tex5) + tex.sample(quadSampler, in.tex2 + offsetX));
	out += inUniform.w[6] * (tex.sample(quadSampler, in.tex6) + tex.sample(quadSampler, in.tex1 + offsetX));
	out += inUniform.w[7] * (tex.sample(quadSampler, in.tex7) + tex.sample(quadSampler, in.tex0 + offsetX));
	
	//out.xyz = out.xyz * 0.5;
	
	out.xyz = mix(tex.sample(quadSampler, in.texcoord).xyz, out.xyz, inUniform.t);
	
	out.w = 1.0;
	
	return out;
	//return float4(1.0, 0.0, 0.0, 1.0);
}

