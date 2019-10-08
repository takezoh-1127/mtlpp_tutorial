//
//  main.cpp
//  tutorial_04
//
//  Created by TAKEZOH on 2019/10/08.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

//
//	テスクチャの読み込みと表示のサンプルプログラム.
//

#include "mtlpp.hpp"
#include "textureloader.hpp"
#include "window.hpp"


constexpr uint32_t SCREEN_WIDTH = 640;
constexpr uint32_t SCREEN_HEIGHT = 480;

mtlpp::Device g_device;
mtlpp::CommandQueue g_commandQueue;
mtlpp::VertexDescriptor g_vertexDesc;
mtlpp::Buffer g_vertexBuffer;
mtlpp::Buffer g_indexBuffer;
mtlpp::RenderPipelineState g_renderPipelineState;
mtlpp::Texture g_texture;


// 頂点データ.
typedef struct
{
	float position[3];
	float texcoord[2];
} VertexData;


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
		
		// 面の向き.
		//	左手系はCW（時計回り）が表.
		//encoder.SetFrontFacingWinding(mtlpp::Winding::CounterClockwise);
		encoder.SetFrontFacingWinding(mtlpp::Winding::Clockwise);
		
		// カリングモード.
		//	Back : 裏面を消す.
		encoder.SetCullMode(mtlpp::CullMode::Back);
		
		encoder.SetVertexBuffer(g_vertexBuffer, 0, 0);
		encoder.SetVertexBuffer(g_vertexBuffer, 0, 1);
		encoder.SetFragmentTexture(g_texture, 0);
		
		encoder.DrawIndexed(mtlpp::PrimitiveType::TriangleStrip, 4, mtlpp::IndexType::UInt32, g_indexBuffer, 0);
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
	g_commandQueue = g_device.NewCommandQueue();
	assert(g_commandQueue);
		
	// シェーダーの初期化.
	mtlpp::Library library = g_device.NewDefaultLibrary();
	assert(library);
		
	mtlpp::Function vertFunc = library.NewFunction("vertFunc");
	assert(vertFunc);
		
	mtlpp::Function fragFunc = library.NewFunction("fragFunc");
	assert(fragFunc);
		
	// テクスチャの読み込み.
	// とりあえずは読み込めている模様...
	{
		mtlpp::TextureLoader textureLoader(g_device);
		assert(textureLoader);
			
		//mtlpp::Texture texture = textureLoader.NewTextureWithPath("data/ColorMap.png");
		mtlpp::Texture texture = textureLoader.NewTextureWithPath("data/Plane.png");
		assert(texture);
			
		g_texture = texture;
	}
		
	// 頂点バッファの定義.
	{
		// シェーダーの頂点の定義の [[attribute(n)]] に対応した設定を行う.
		mtlpp::VertexDescriptor desc;
		
		auto attributes = desc.GetAttributes();
		auto layouts = desc.GetLayouts();
		
		// position.
		{
			auto attr = attributes[0];
					
			attr.SetFormat(mtlpp::VertexFormat::Float3);
			attr.SetOffset(0);
			attr.SetBufferIndex(0);
		}
				
		// texcoord.
		{
			auto attr = attributes[1];
					
			attr.SetFormat(mtlpp::VertexFormat::Float2);
			//attr.SetOffset(0);	// SetVertexBufferのオフセットでも指定できる.
			attr.SetOffset(12);		// texcoordのオフセット(byte).
			attr.SetBufferIndex(1);
		}
				
		{
			auto layout = layouts[0];
					
			layout.SetStride(20);	// 頂点バッファのサイズ. float(4byte) * 3 + float(4byte) * 2
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}
				
		{
			auto layout = layouts[1];
					
			layout.SetStride(20);	// 頂点バッファのサイズ. float(4byte) * 3 + float(4byte) * 2
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}

		g_vertexDesc = desc;
	}
		
	// 頂点バッファの生成.
	// uv
	//	左上 : 0, 0
	//	左下 : 0, 1
	//	右上 : 1, 0
	//	右下 : 1, 1
	{
		VertexData v[] =
		{
#if 0
			{ {-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },		// 左上.
			{ {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },		// 右上.
			{ {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },		// 左下.
			{ {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },		// 右下.
			
			//{ {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },		// 右上.
			//{ {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },		// 左下.
#else
			{ {-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f} },		// 左上.
			{ {0.5f, 0.5f, 0.0f}, {1.0f, 0.0f} },		// 右上.
			{ {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f} },		// 左下.
			{ {0.5f, -0.5f, 0.0f}, {1.0f, 1.0f} },		// 右下.
			
			//{ {0.5f, 0.5f, 0.0f}, {1.0f, 0.0f} },		// 右上.
			//{ {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f} },		// 左下.
#endif
		};
			
		g_vertexBuffer = g_device.NewBuffer(v, sizeof(v), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
		assert(g_vertexBuffer);
	}
	
	// インデックスバッファの生成.
	{
		uint32_t index[] =
		{
			//0, 1, 2,
			0, 1, 2, 3,
		};
			
		g_indexBuffer = g_device.NewBuffer(index, sizeof(index), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
		assert(g_indexBuffer);
	}
		
	// レンダーパイプラインの生成.
	{
		mtlpp::RenderPipelineDescriptor desc;
			
		desc.SetVertexFunction(vertFunc);
		desc.SetFragmentFunction(fragFunc);
			
		desc.SetVertexDescriptor(g_vertexDesc);
			
		auto attach = desc.GetColorAttachments();
		attach[0].SetPixelFormat(mtlpp::PixelFormat::BGRA8Unorm);
			
		g_renderPipelineState = g_device.NewRenderPipelineState(desc, nullptr);
		assert(g_renderPipelineState);
	}
		
	//
	Window window(g_device, &Render, SCREEN_WIDTH, SCREEN_HEIGHT);
		
	Window::Run();
	
	return 0;
}
