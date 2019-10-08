//
//  main.cpp
//  tutorial_01
//
//  Created by TAKEZOH on 2019/09/04.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

//
//	三角形を表示するサンプルプログラム.
//
//	MSL(Metal Sharding Language)を *.metal ファイルで用意して
//	プロジェクトに追加して使用するサンプルプログラム.
//
//	basic.metal
//	basic2.metal
//	複数のシェーダーファイルをビルドした場合はそれぞれの関数名で取得できる.
//	関数名が重複している場合はビルドエラーになる.
//


#include "mtlpp.hpp"
#include "window.hpp"


mtlpp::Device g_device;
mtlpp::CommandQueue g_commandQueue;
mtlpp::Buffer g_vertexBuffer;
mtlpp::RenderPipelineState g_renderPipelineState;


/**
	Renderと言いつつUpdateとかの処理も書くのでdoFrameとかの方が良いかも...
*/
void Render(const Window& window)
{
	mtlpp::CommandBuffer commandBuffer = g_commandQueue.CommandBuffer();
	assert(commandBuffer.GetPtr());
	
	mtlpp::RenderPassDescriptor desc = window.GetRenderPassDescriptor();
	
	if(desc)
	{
		mtlpp::RenderCommandEncoder encoder = commandBuffer.RenderCommandEncoder(desc);
		assert(encoder);
		
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
	assert(g_device);
	
	// コマンドキューの生成.
	g_commandQueue = g_device.NewCommandQueue();
	assert(g_commandQueue);
	
	// シェーダーの初期化.
	// プロジェクトに組み込んでビルドした場合はデフォルトで取得できるようになる.
	mtlpp::Library library = g_device.NewDefaultLibrary();
	assert(library);
	
	mtlpp::Function vertFunc = library.NewFunction("vertFunc");
	assert(vertFunc);
	
	mtlpp::Function fragFunc = library.NewFunction("fragFunc");
	assert(fragFunc);
	
#if 0
	// basic2.metalの関数も取得できる.
	mtlpp::Function vertFunc2 = library.NewFunction("vertFunc2");
	assert(vertFunc2);
	
	mtlpp::Function fragFunc2 = library.NewFunction("fragFunc2");
	assert(fragFunc2);
#endif
	
	// 頂点バッファの生成.
	{
		const float vertexData[] =
		{
			0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
		};
		
		g_vertexBuffer = g_device.NewBuffer(vertexData, sizeof(vertexData), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
		assert(g_vertexBuffer);
	}
	
	// レンダーパイプラインの生成.
	{
		mtlpp::RenderPipelineDescriptor desc;
		
		desc.SetVertexFunction(vertFunc);
		desc.SetFragmentFunction(fragFunc);
		
		auto attach = desc.GetColorAttachments();
		attach[0].SetPixelFormat(mtlpp::PixelFormat::BGRA8Unorm);
		
		g_renderPipelineState = g_device.NewRenderPipelineState(desc, nullptr);
		assert(g_renderPipelineState);
	}
	
	Window window(g_device, &Render, 320, 240);
	
	Window::Run();
	
	return 0;
}
