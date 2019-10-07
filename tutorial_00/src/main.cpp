//
//  main.cpp
//  tutorial_00
//
//  Created by TAKEZOH on 2019/09/02.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

//
//	三角形を表示するサンプルプログラム.
//

//#include <iostream>

#include "mtlpp.hpp"
#include "window.hpp"


mtlpp::Device g_device;
mtlpp::CommandQueue g_commandQuene;
mtlpp::Buffer g_vertexBuffer;
mtlpp::RenderPipelineState g_renderPipelineState;


/**
*/
void Render(const Window& window)
{
	//printf("### render()\n");
	
	mtlpp::CommandBuffer commandBuffer = g_commandQuene.CommandBuffer();
	assert(commandBuffer.GetPtr());
	
	mtlpp::RenderPassDescriptor renderPassDesc = window.GetRenderPassDescriptor();
	
	if(renderPassDesc)
	{
		mtlpp::RenderCommandEncoder encoder = commandBuffer.RenderCommandEncoder(renderPassDesc);
		assert(encoder.GetPtr());
		
		encoder.SetRenderPipelineState(g_renderPipelineState);
		encoder.SetVertexBuffer(g_vertexBuffer, 0, 0);
		encoder.Draw(mtlpp::PrimitiveType::Triangle, 0, 3);
		encoder.EndEncoding();
		
		commandBuffer.Present(window.GetDrawable());
	}
	
	commandBuffer.Commit();
	commandBuffer.WaitUntilCompleted();
}


//int main(int argc, const char * argv[])
int main()
{
	// デバイスの生成.
	g_device = mtlpp::Device::CreateSystemDefaultDevice();
	assert(g_device.GetPtr());
	
	// コマンドキューの生成.
	g_commandQuene = g_device.NewCommandQueue();
	assert(g_commandQuene.GetPtr());
	
	// Rは生文字列リテラル.
	const char shaderSrc[] = R"""(
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
			return half4(1.0, 0.0, 0.0, 1.0);
		}
	)""";
	
	// ns::Errorのインスタンスを作るときにエラーになってしまう...
	//ns::Error* error = nullptr;
	
	// シェーダーの初期化.
	// シェーダーの生成に失敗したらシェーダーのビルドエラーを疑う.
	// わからない時はシェーダーファイルに書いてビルドしてみる.
	//mtlpp::Library library = g_device.NewDefaultLibrary();
	mtlpp::Library library = g_device.NewLibrary(shaderSrc, mtlpp::CompileOptions(), nullptr);
	
	//printf("###error code:[%d]\n,", error.GetCode());
	//printf("###domain : [%s]\n", error.GetDomain().GetCStr());
	//printf("###help : [%s]\n,", error.GetHelpAnchor().GetCStr());
	
	assert(library);
	//assert(library.GetPtr());
	
	mtlpp::Function vertFunc = library.NewFunction("vertFunc");
	assert(vertFunc.GetPtr());
	
	mtlpp::Function fragFunc = library.NewFunction("fragFunc");
	assert(fragFunc.GetPtr());
	
	// 頂点バッファの生成.
	{
		const float vertexData[] =
		{
			0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
		};
	
		g_vertexBuffer = g_device.NewBuffer(vertexData, sizeof(vertexData), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
		assert(g_vertexBuffer.GetPtr());
	}

	// レンダーパイプラインの生成.
	{
		mtlpp::RenderPipelineDescriptor desc;
	
		desc.SetVertexFunction(vertFunc);
		desc.SetFragmentFunction(fragFunc);
		
		auto attach = desc.GetColorAttachments();
		attach[0].SetPixelFormat(mtlpp::PixelFormat::BGRA8Unorm);
	
		g_renderPipelineState = g_device.NewRenderPipelineState(desc, nullptr);
		assert(g_renderPipelineState.GetPtr());
	}
	
	Window window(g_device, &Render, 320, 240);
	
	Window::Run();
	
	return 0;
}
