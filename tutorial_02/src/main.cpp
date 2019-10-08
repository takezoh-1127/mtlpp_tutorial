//
//  main.cpp
//  tutorial_02
//
//  Created by TAKEZOH on 2019/09/05.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

//
//	三角形を表示するサンプルプログラム.
//
//	MSL(Metal Sharding Language)の *.metal ファイルをコマンドラインでビルドして *.metallib を作成して
//	そのファイルを読み込んでシェーダーを使用するサンプルプログラム.
//
//	・metalファイル
//		basic.metal
//		basic2.metal
//
//	・metallibのビルド方法
//		ターミナルなどから以下のコマンドを実行する.
//
//		*.metalをコンパイルして中間フォーマットの *.air を作成.
//			$ xcrun -sdk macosx metal basic.metal -c -o basic.air
//
//		*.metallibをビルドする.
//		複数の *.air をまとめてビルドすることも可能.
//		その際、関数名が重複してしまうとビルドエラーになってしまうので注意が必要.
//			$ xcrun -sdk macosx metallib basic.air basic2.air -o basic.metallib
//
//	・ワーキングディレクトリの設定
//		プロジェクトファイル直下にあるリソースを読み込むためにワーキングディレクトリの設定を行う.
//		メインメニューの [Product] -> [Scheme] -> [Edit Scheme] の Run の Optionタブの Working Directory の Use custom working directory: にチェックを入れる.
//		項目に $(SRCROOT) と入力.
//


#include "mtlpp.hpp"
#include "window.hpp"


mtlpp::Device g_device;
mtlpp::CommandQueue g_commandQueue;
mtlpp::Buffer g_vertexBuffer;
mtlpp::RenderPipelineState g_renderPipelineState;


/**
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
int main(int argc, const char * argv[])
{
	// デバイスの生成.
	g_device = mtlpp::Device::CreateSystemDefaultDevice();
	assert(g_device);
	
	// コマンドキューの生成.
	g_commandQueue = g_device.NewCommandQueue();
	assert(g_commandQueue);
	
	// シェーダーの初期化.
	// パス指定だとMetalのライブラリで勝手に読み込んです初期化される.
	// 上位でよ読み込んでバッファを渡して初期化は newLibraryWithData を使うとできそうだがmtlppではそれを使ったAPIが用意されていないので拡張する必要が出てくる.
	mtlpp::Library library = g_device.NewLibrary("shader/basic.metallib", nullptr);
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
