//
//  main.cpp
//  tutorial_07
//
//  Created by TAKEZOH on 2019/12/19.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

//
//	立方体の表示のサンプルプログラム.
//	tutorial_05のlambertシェーダーをローカルスペースに書き換えたもの.
//

//#include <iostream>
#include <string>
#include <chrono>

#include "mtlpp.hpp"
#include "textureloader.hpp"
#include "window.hpp"

#include "Math.h"
#include "Transform.h"

constexpr uint32_t SCREEN_WIDTH = 640;
constexpr uint32_t SCREEN_HEIGHT = 480;

mtlpp::Device g_device;
mtlpp::CommandQueue g_commandQueue;
mtlpp::RenderPipelineState g_renderPipelineState;
mtlpp::DepthStencilState g_depthState;

mtlpp::VertexDescriptor g_vertexDesc;
mtlpp::Buffer g_vertexBuff;
mtlpp::Buffer g_indexBuff;

mtlpp::Texture g_texture;

mtlpp::Buffer g_uniformBuff;

// 頂点データ.
typedef struct
{
	float position[3];
	float normal[3];
	float texcoord[2];
}VertexData;

// 頂点シェーダーに渡すユニフォームバッファ.
typedef struct
{
	Math::Matrix4 VP;				// view -> projection
	Math::Matrix4 LW;				// local -> world
	Math::Matrix4 WVP;				// local -> world -> view -> projection
	Math::Matrix4 invLW;			// invert LW
	Math::Vector3 lightDir;
	//float pad0;
	Math::Vector3 cameraPos;
	//float pad1;
	Math::Vector3 diffuseColor;
	Math::Vector3 specularColor;
	Math::Vector3 ambientColor;
	float specularPower;
}UniformBuffer;


std::chrono::system_clock::time_point g_tickCount;

Transform g_transform;

/**
*/
void doFrame(const Window& window)
{
	// この辺りはTimerなどで隠蔽、まとめる予定.
	auto now = std::chrono::system_clock::now();
	//float deltaTime = ((std::chrono::duration_cast<std::chrono::microseconds>(now - g_tickCount).count()) / (1000.0 * 1000.0));
	float deltaTime = ((std::chrono::duration_cast<std::chrono::milliseconds>(now - g_tickCount).count()) / (1000.0f));
	g_tickCount = std::chrono::system_clock::now();
	
	constexpr float MAX_DELTA_TIME = (1.0f / 60.0f) * 5.0f;
	
	if(deltaTime > MAX_DELTA_TIME)
	{
		deltaTime = MAX_DELTA_TIME;
	}
	
	
	// カメラ処理.
	Math::Matrix4 projection;
	Math::Matrix4 view;
	
	{
		float fovY = Math::ToRadians(60.0f);
		float width = static_cast<float>(SCREEN_WIDTH);
		float height = static_cast<float>(SCREEN_HEIGHT);
		float near = 1.0f;
		float far = 10000.0f;
		
		projection = Math::Matrix4::CreatePerspectiveFOV(fovY, width, height, near, far);
	}
	
	{
		Math::Vector3 cameraPos = Math::Vector3(0.0f, 0.0f, -4.0f);
		Math::Vector3 target = cameraPos + Math::Vector3::UnitZ() * 1.0f;
		//Math::Vector3 cameraPos = Math::Vector3(5.0f, 0.0f, 0.0f);
		//Math::Vector3 target = cameraPos + Math::Vector3::UnitX() * -1.0f;
		Math::Vector3 up = Math::Vector3::UnitY();
		
		view = Math::Matrix4::CreateLookAt(cameraPos, target, up);
	}
	
	// モデルのマトリクスの更新.
	{
		float angle = -1.0f * deltaTime;
	
		//g_position.x = 2.0f;
	
		Math::Quaternion incY(Math::Vector3::UnitY(), angle * 0.5f);
		Math::Quaternion incX(Math::Vector3::UnitX(), angle * 0.2f);
		//Math::Quaternion incX(Math::Vector3::UnitX(), Math::ToRadians(45.0f*0.5f));
		
		auto rot = g_transform.GetRotation();
		
		rot = Math::Quaternion::Concatenate(rot, incY);
		rot = Math::Quaternion::Concatenate(rot, incX);
		
		//rot = incX;
		
		g_transform.SetRotation(rot);
		
		//g_transform[0].SetPosition(Math::Vector3(2.0f,0.0f,0.0f));
	}
	
	
#if 1
	// ユニフォームバッファの設定.
	{
		UniformBuffer* buffer = static_cast<UniformBuffer*>(g_uniformBuff.GetContents());
		
		buffer->VP = view * projection;
	
		buffer->LW = g_transform.GetWorldMatrix();
		buffer->invLW = buffer->LW;
		buffer->invLW.Invert();
		
		buffer->WVP = g_transform.GetWorldMatrix() * view * projection;
		//buffer->WVP = view * projection * g_transform.GetWorldMatrix();
		
		// ライトの向きをローカル座標系に変換.
		{
			auto lightDir = Math::Vector3::Normalize(Math::Vector3(0.0f, -0.7f, 0.7f));
			
			auto invLW = buffer->invLW;
			
			//buffer->lightDir = Math::Vector3::Normalize(Math::Vector3(0.7f, -0.7f, 0.7f));
			buffer->lightDir = Math::Vector3::Transform(lightDir, invLW, 0.0f);
		}
		
		// 光源色を頂点シェーダーに渡す.
		{
			buffer->ambientColor = Math::Vector3(0.4f, 0.4f, 0.4f);
			//buffer->ambientColor = Math::Vector3(0.0f, 0.0f, 0.0f);
		
			//buffer->lightDir = Math::Vector3::Normalize(Math::Vector3(0.7f, -0.7f, 0.7f));
			//buffer->lightDir = Math::Vector3::Normalize(Math::Vector3(0.0f, -0.7f, 0.7f));
			//buffer->lightDir = Math::Vector3::Normalize(Math::Vector3(0.0f, 0.0f, 1.0f));
			buffer->diffuseColor = Math::Vector3(1.0f, 1.0f, 1.0f);
			buffer->specularColor = Math::Vector3(0.6f, 0.6f, 0.6f);
			
			//buffer->specularPower = 25.0f;
		}
	}
#endif

	//
	mtlpp::CommandBuffer commandBuffer = g_commandQueue.CommandBuffer();
	assert(commandBuffer);
		
	mtlpp::RenderPassDescriptor desc = window.GetRenderPassDescriptor();
		
	if(desc)
	{
		mtlpp::RenderCommandEncoder encoder = commandBuffer.RenderCommandEncoder(desc);
		assert(encoder);
			
		encoder.SetRenderPipelineState(g_renderPipelineState);
			
		encoder.SetDepthStencilState(g_depthState);
			
		// 面の向き.
		// 左手系はCW（時計周り）が表.
		encoder.SetFrontFacingWinding(mtlpp::Winding::Clockwise);
			
		// カリングモード.
		encoder.SetCullMode(mtlpp::CullMode::Back);
			
		{
			// ユニフォームバッファをセット.
			encoder.SetVertexBuffer(g_uniformBuff, 0, 3);
			//encoder.SetFragmentBuffer(g_phongUniformBuff, 0, 3);
			
			// 頂点バッファをセット.
			encoder.SetVertexBuffer(g_vertexBuff, 0, 0);
			encoder.SetVertexBuffer(g_vertexBuff, 0, 1);
			encoder.SetVertexBuffer(g_vertexBuff, 0, 2);
			//encoder.SetVertexBuffer(g_vertexBuff, 0, 3);
			
			// テクスチャをセット.
			encoder.SetFragmentTexture(g_texture, 0);
			//encoder.SetFragmentTexture(g_texture, 1);
			
			// インデックスバッファによる描画.
			encoder.DrawIndexed(mtlpp::PrimitiveType::Triangle, 36, mtlpp::IndexType::UInt32, g_indexBuff, 0);
		}
			
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
	mtlpp::Library library = g_device.NewDefaultLibrary();
	assert(library);
	
	mtlpp::Function vertFunc = library.NewFunction("lambertVS");
	assert(vertFunc);
	
	mtlpp::Function fragFunc = library.NewFunction("lambertFS");
	assert(fragFunc);
	
	// テクスチャの読み込み.
	{
		//std::string fileName = "data/Plane.png";
		std::string fileName = "data/brick_4_diff_1k.png";
		//std::string fileName = "data/brick_4_nor_1k.png";
		
		mtlpp::TextureLoader textureLoader(g_device);
		assert(textureLoader);
	
		mtlpp::Texture texture = textureLoader.NewTextureWithPath(fileName.c_str());
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
			
			// x,y,z
			attr.SetFormat(mtlpp::VertexFormat::Float3);
			attr.SetOffset(0);
			attr.SetBufferIndex(0);
		}
		
		// normal.
		{
			auto attr = attributes[1];
			
			// x,y,z
			attr.SetFormat(mtlpp::VertexFormat::Float3);
			//attr.SetOffset(0);	// SetVertexBufferのオフセットでも指定できる模様...
			attr.SetOffset(12);		// 先頭からのオフセット(byte).
			attr.SetBufferIndex(1);
		}
		
		// texcoord.
		{
			auto attr = attributes[2];
			
			// u,v
			attr.SetFormat(mtlpp::VertexFormat::Float2);
			attr.SetOffset(24);
			attr.SetBufferIndex(2);
		}
		
		// position.
		{
			auto layout = layouts[0];
			
			layout.SetStride(32);	// 頂点バッファのサイズ. float(4byte) * 3 + float(4byte) * 3 + float(4byte) * 2
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}
		
		// normal
		{
			auto layout = layouts[1];
			
			layout.SetStride(32);
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}
		
		// texcoord.
		{
			auto layout = layouts[2];
			
			layout.SetStride(32);
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}
		
		g_vertexDesc = desc;
	}
	
	// 頂点バッファの生成.
	{
		VertexData vertex[] =
		{
			// 手前.
			{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f} },
			{ {-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f} },
			{ {1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f} },
			{ {1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} },
			
			// 右.
			{ {1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
			{ {1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			{ {1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
			{ {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
			
			// 奥.
			{ {1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },
			{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} },
			{ {-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
			{ {-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
			
			// 左.
			{ {-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
			{ {-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			{ {-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
			{ {-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
			
			// 上.
			{ {-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
			{ {-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },
			{ {1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} },
			{ {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },
			
			// 下.
			{ {-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },
			{ {-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f} },
			{ {1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },
			{ {1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },
		};
		
		g_vertexBuff = g_device.NewBuffer(vertex, sizeof(vertex), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
		assert(g_vertexBuff);
	}
	
	// インデックスバッファの生成.
	{
		int32_t index[] =
		{
			// 手前.
			0, 1, 2,
			2, 1, 3,
			
			// 右.
			4, 5, 6,
			6, 5, 7,
			
			// 奥.
			8, 9, 10,
			10, 9, 11,
			
			// 左.
			12, 13, 14,
			14, 13, 15,
			
			// 上.
			16, 17, 18,
			18, 17, 19,
			
			// 下.
			20, 21, 22,
			22, 21, 23,
		};
		
		g_indexBuff = g_device.NewBuffer(index, sizeof(index), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
		assert(g_indexBuff);
	}
	
	// レンダーパイプラインの生成.
	{
		mtlpp::RenderPipelineDescriptor desc;
		
		desc.SetVertexFunction(vertFunc);
		desc.SetFragmentFunction(fragFunc);
		
		// インデックスバッファの場合は頂点ディスクリプタを設定する必要がある？.
		desc.SetVertexDescriptor(g_vertexDesc);
		
		auto attach = desc.GetColorAttachments();
		attach[0].SetPixelFormat(mtlpp::PixelFormat::BGRA8Unorm);
		
		desc.SetDepthAttachmentPixelFormat(mtlpp::PixelFormat::Depth32Float_Stencil8);
		desc.SetStencilAttachmentPixelFormat(mtlpp::PixelFormat::Depth32Float_Stencil8);
		
		g_renderPipelineState = g_device.NewRenderPipelineState(desc, nullptr);
		assert(g_renderPipelineState);
	}
	
	// Depthステートの生成.
	{
		mtlpp::DepthStencilDescriptor desc;
		
		desc.SetDepthCompareFunction(mtlpp::CompareFunction::Less);
		desc.SetDepthWriteEnabled(true);
		
		g_depthState = g_device.NewDepthStencilState(desc);
		assert(g_depthState);
	}
	
	// ユニフォームバッファの生成.
	{
		g_uniformBuff = g_device.NewBuffer(sizeof(UniformBuffer), mtlpp::ResourceOptions::StorageModeShared);
		assert(g_uniformBuff);
	}
	
	
	Window window(g_device, &doFrame, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	Window::Run();
	
	return 0;
}
