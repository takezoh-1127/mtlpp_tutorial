//
//  basic.metal
//  tutorial_01
//
//  Created by TAKEZOH on 2019/09/04.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;


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
