//
//  main.cpp
//  NormalMapping
//
//  Created by TAKEZOH on 2020/04/07.
//  Copyright © 2020 TAKEZOH. All rights reserved.
//

//
//	ノーマルマップのサンプルプログラム.
//
//	頂点座標系での計算.
//	頂点座標系は頂点ごとに計算した接ベクトルを規定ベクトルとした座標系.
//
//	uvは左上が(0, 0)、右下が(1, 1)
//	Z軸が法線ベクトル(N:normal)
//	X軸が接ベクトル(T:tangent)
//	Y軸が従法線ベクトル(B:binormal)
//
//	接ベクトル空間、タンジェント空間とも呼ばれる.
//
//	頂点座標系のメリットは「法線マップがそのまま法線の向きになる」.
//
//	ローカル座標系から頂点座標系への変換
//	接ベクトルと従法線ベクトルを計算することができれば、それぞれの基底ベクトルから変換行列を計算することができる.
//	接ベクトルTはテクスチャ座標の横方向uが変化したときの位置座標の変化.
//	従法線ベクトルBはテクスチャ座標の縦方向vが変化したときの位置座標の変化.
//
//
//	実際の計算手順
//		頂点空間座標系の基底ベクトルを計算
//		頂点空間の基底ベクトルを使って、視線ベクトルや光源ベクトルをローカル座標系から頂点座標系に変換.
//



//#include <iostream>
#include <string>

#include "mtlpp.hpp"
#include "textureloader.hpp"

#include "Timer.h"

#include "Math.h"
#include "Transform.h"
//#include "StringUtility.h"

#include "GfxCamera.h"

#include "window.hpp"


constexpr uint32_t SCREEN_WIDTH = 640;
constexpr uint32_t SCREEN_HEIGHT = 480;

mtlpp::Device g_device;

mtlpp::CommandQueue g_commandQueue;

mtlpp::RenderPipelineState g_renderPipelineState;
mtlpp::DepthStencilState g_depthState;

mtlpp::VertexDescriptor g_vertexDesc;

mtlpp::Texture g_diffuseMap;
mtlpp::Texture g_normalMap;


mtlpp::Buffer g_vertexBuff;
mtlpp::Buffer g_indexBuff;
int32_t g_vertexNum = 0;
int32_t g_indexNum = 0;

mtlpp::Buffer g_uniformBuff;


Timer g_frameCount;


Transform g_transform;

Gfx::Camera g_camera;

// 頂点データ.
typedef struct
{
	float position[3];
	float normal[3];
	float tangent[3];
	float binormal[3];
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
	Math::Vector3 cameraPos;
	Math::Vector3 diffuseColor;
	Math::Vector3 specularColor;
	Math::Vector3 ambientColor;
	float specularPower;
}UniformBuffer;


/**
 */
void update(float deltaT)
{
	// 通常シーンのビューポート.
	{
		Math::Vector3 cameraPos = Math::Vector3(0.0f, 0.0f, -5.0f);
		//Math::Vector3 cameraPos = Math::Vector3(0.0f, 7.0f, -25.0f);
		Math::Vector3 target = cameraPos + Math::Vector3::UnitZ() * 1.0f;
		//Math::Vector3 cameraPos = Math::Vector3(5.0f, 0.0f, 0.0f);
		//Math::Vector3 target = cameraPos + Math::Vector3::UnitX() * -1.0f;
		Math::Vector3 up = Math::Vector3::UnitY();
				
		Math::Matrix4 view = Math::Matrix4::CreateLookAt(cameraPos, target, up);
		
		g_camera.SetView(view);
	}
	
	// モデルのマトリクスの更新.
	{
		float angle = 1.0f * deltaT;
		
		//g_position.x = 2.0f;
		
		Math::Quaternion incY(Math::Vector3::UnitY(), angle * 0.5f);
		//Math::Quaternion incY(Math::Vector3::UnitY(), Math::ToRadians(180.0f));
		Math::Quaternion incX(Math::Vector3::UnitX(), angle * 0.2f);
		//Math::Quaternion incX(Math::Vector3::UnitX(), Math::ToRadians(45.0f*0.5f));
			
		auto rot = g_transform.GetRotation();
			
		rot = Math::Quaternion::Concatenate(rot, incY);
		rot = Math::Quaternion::Concatenate(rot, incX);
		
		//rot = incY;
		//rot = incX;
			
		g_transform.SetRotation(rot);
			
		//g_transform[0].SetPosition(Math::Vector3(2.0f,0.0f,0.0f));
	}
	
	// ユニフォームバッファの設定はRender側か？.
	// ユニフォームバッファの設定.
	{
		Math::Matrix4 projection = g_camera.GetProjection();
		Math::Matrix4 view = g_camera.GetView();
		
		UniformBuffer* buffer = static_cast<UniformBuffer*>(g_uniformBuff.GetContents());
				
		buffer->VP = view * projection;
			
		buffer->LW = g_transform.GetWorldMatrix();
		buffer->invLW = buffer->LW;
		buffer->invLW.Invert();
				
		buffer->WVP = g_transform.GetWorldMatrix() * view * projection;
		//buffer->WVP = view * projection * g_transform.GetWorldMatrix();
		
#if 0
		// ワールドスペース.
		{				
			//buffer->cameraPos = view.GetTranslation();
			
			//auto invView = g_camera.GetInvView();
			//auto pos = invView.GetTranslation();
			
			//printf(">>> camera pos:[%f][%f][%f]\n",pos.GetX(),pos.GetY(),pos.GetZ());
			
			//buffer->cameraPos = pos;
			//buffer->cameraPos = g_camera.GetPostion();
			buffer->cameraPos = Math::Vector3(0.0f, 0.0f, -5.0f);
		}
#else
		// 視点をローカル座標系に変換.
		{
			auto WV = g_transform.GetWorldMatrix() * view;
				
			WV.Invert();
				
			buffer->cameraPos = WV.GetTranslation();
		}
#endif
			
		// ライトの向きをローカル座標系に変換.
		{
			auto lightDir = Math::Vector3::Normalize(Math::Vector3(0.7f, -0.7f, 0.7f));
			//auto lightDir = Math::Vector3::Normalize(Math::Vector3(0.0f, -0.7f, 0.0f));
			
#if 0
			// ワールドスペース.
			buffer->lightDir = Math::Vector3::Normalize(lightDir);
#else
			// ローカルスペース.
			auto invLW = buffer->invLW;
			
			buffer->lightDir = Math::Vector3::Transform(lightDir, invLW, 0.0f);
#endif
		}
				
		// 光源色を頂点シェーダーに渡す.
		{
			buffer->ambientColor = Math::Vector3(0.4f, 0.4f, 0.4f);
			//buffer->ambientColor = Math::Vector3(0.0f, 0.0f, 0.0f);
				
			buffer->diffuseColor = Math::Vector3(1.0f, 1.0f, 1.0f);
			buffer->specularColor = Math::Vector3(0.5f, 0.5f, 0.5f);
					
			buffer->specularPower = 40.0f;
		}
	}
}

/**
 */
void render(const Window& window)
{
	{
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
				// ユニフォームバッファ、頂点バッファとと共にSetVertexBufferで渡すのでインデックスがバッティングしないように注意が必要.
				encoder.SetVertexBuffer(g_uniformBuff, 0, 5);
				//encoder.SetFragmentBuffer(g_phongUniformBuff, 0, 3);
					
				// 頂点バッファをセット.
				encoder.SetVertexBuffer(g_vertexBuff, 0, 0);
				encoder.SetVertexBuffer(g_vertexBuff, 0, 1);
				encoder.SetVertexBuffer(g_vertexBuff, 0, 2);
				encoder.SetVertexBuffer(g_vertexBuff, 0, 3);
				encoder.SetVertexBuffer(g_vertexBuff, 0, 4);
					
				// テクスチャをセット.
				encoder.SetFragmentTexture(g_diffuseMap, 0);
				encoder.SetFragmentTexture(g_normalMap, 1);
					
				// インデックスバッファによる描画.
				encoder.DrawIndexed(mtlpp::PrimitiveType::Triangle, g_indexNum, mtlpp::IndexType::UInt32, g_indexBuff, 0);
			}
					
			encoder.EndEncoding();
		}
		
		commandBuffer.Commit();
	}
	
	{
		mtlpp::CommandBuffer commandBuffer = g_commandQueue.CommandBuffer();
		assert(commandBuffer);
		
		commandBuffer.Present(window.GetDrawable());
		
		commandBuffer.Commit();
		commandBuffer.WaitUntilCompleted();
	}
}

/**
*/
void doFrame(const Window& window)
{
	g_frameCount = Timer::GetCurrentTime() - g_frameCount;
	
	float deltaTime = g_frameCount.GetSecond();
	
	g_frameCount = Timer::GetCurrentTime();
	
	constexpr float MAX_DELTA_TIME = (1.0f / 60.0f) * 5.0f;
		
	if(deltaTime > MAX_DELTA_TIME)
	{
		deltaTime = MAX_DELTA_TIME;
	}
	
	// 更新.
	update(deltaTime);
	
	// 描画.
	render(window);
}

//int main(int argc, const char * argv[])
int main()
{
	{
		//printf(">>> vertex data size : [%d]\n",sizeof(VertexData));
	}
	
	// デバイスの生成.
	g_device = mtlpp::Device::CreateSystemDefaultDevice();
	assert(g_device);
	
	// コマンドキューの生成.
	g_commandQueue = g_device.NewCommandQueue();
	assert(g_commandQueue);
	
	// シェーダーの初期化.
	mtlpp::Library library = g_device.NewDefaultLibrary();
	assert(library);
	
	mtlpp::Function normalMappingVS = library.NewFunction("normalMappingVS");
	assert(normalMappingVS);
	
	mtlpp::Function normalMappingFS = library.NewFunction("normalMappingFS");
	assert(normalMappingFS);
	
	
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
		
		// tangent.
		{
			auto attr = attributes[2];
			
			// x,y,z
			attr.SetFormat(mtlpp::VertexFormat::Float3);
			//attr.SetOffset(0);	// SetVertexBufferのオフセットでも指定できる模様...
			attr.SetOffset(24);		// 先頭からのオフセット(byte).
			attr.SetBufferIndex(2);
		}
		
		// binormal.
		{
			auto attr = attributes[3];
			
			// x,y,z
			attr.SetFormat(mtlpp::VertexFormat::Float3);
			//attr.SetOffset(0);	// SetVertexBufferのオフセットでも指定できる模様...
			attr.SetOffset(36);		// 先頭からのオフセット(byte).
			attr.SetBufferIndex(3);
		}
		
		// texcoord.
		{
			auto attr = attributes[4];
			
			// u,v
			attr.SetFormat(mtlpp::VertexFormat::Float2);
			attr.SetOffset(48);
			attr.SetBufferIndex(4);
		}
		
		// position.
		{
			auto layout = layouts[0];
			
			layout.SetStride(56);	// 頂点バッファのサイズ. float(4byte) * 3 + float(4byte) * 3 + float(4byte) * 3 + float(4byte) * 3 + float(4byte) * 2
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}
		
		// normal
		{
			auto layout = layouts[1];
			
			layout.SetStride(56);
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}
		
		// tangent
		{
			auto layout = layouts[2];
			
			layout.SetStride(56);
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}
		
		// binormal
		{
			auto layout = layouts[3];
			
			layout.SetStride(56);
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}
		
		// texcoord.
		{
			auto layout = layouts[4];
			
			layout.SetStride(56);
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}
		
		g_vertexDesc = desc;
	}
	
	{
		VertexData vertex[] =
		{
			// 手前.
			{ {-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
			{ {-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			{ {1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
			{ {1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
			
			// 右.
			{ {1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
			{ {1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			{ {1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
			{ {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
			
			// 奥.
			{ {1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
			{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			{ {-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
			{ {-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
			
			// 左.
			{ {-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
			{ {-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			{ {-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
			{ {-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
			
			// 上.
			{ {-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
			{ {-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			{ {1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
			{ {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
			
			// 下.
			{ {-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f} },
			{ {-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			{ {1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f} },
			{ {1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
		};
		
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
		
		// tangent,binormalの計算.
		{
			for(int32_t i = 0;i < sizeof(index) / sizeof(index[0]);i+=3)
			{
				VertexData& v0 = vertex[index[i+0]];
				VertexData& v1 = vertex[index[i+1]];
				VertexData& v2 = vertex[index[i+2]];
				
				Math::Vector3 p0 = Math::Vector3(v0.position[0], v0.position[1], v0.position[2]);
				Math::Vector3 p1 = Math::Vector3(v1.position[0], v1.position[1], v1.position[2]);
				Math::Vector3 p2 = Math::Vector3(v2.position[0], v2.position[1], v2.position[2]);
				
				Math::Vector2 uv0 = Math::Vector2(v0.texcoord[0], v0.texcoord[1]);
				Math::Vector2 uv1 = Math::Vector2(v1.texcoord[0], v1.texcoord[1]);
				Math::Vector2 uv2 = Math::Vector2(v2.texcoord[0], v2.texcoord[1]);
				
				Math::Vector3 edge0 = p1 - p0;
				Math::Vector3 edge1 = p2 - p0;
				
				Math::Vector2 deltaUV0 = uv1 - uv0;
				Math::Vector2 deltaUV1 = uv2 - uv0;
				
				float f = 1.0f / (deltaUV0.GetX() * deltaUV1.GetY() - deltaUV1.GetX() * deltaUV0.GetY());
				
				{
					float x = f * (deltaUV1.GetY() * edge0.GetX() - deltaUV0.GetY() * edge1.GetX());
					float y = f * (deltaUV1.GetY() * edge0.GetY() - deltaUV0.GetY() * edge1.GetY());
					float z = f * (deltaUV1.GetY() * edge0.GetZ() - deltaUV0.GetY() * edge1.GetZ());
					
					Math::Vector3 tangent0 = Math::Vector3(x, y, z);
					
					tangent0.Normalize();
					
					v0.tangent[0] = tangent0.GetX();
					v0.tangent[1] = tangent0.GetY();
					v0.tangent[2] = tangent0.GetZ();
					
					v1.tangent[0] = tangent0.GetX();
					v1.tangent[1] = tangent0.GetY();
					v1.tangent[2] = tangent0.GetZ();
					
					v2.tangent[0] = tangent0.GetX();
					v2.tangent[1] = tangent0.GetY();
					v2.tangent[2] = tangent0.GetZ();
				}
				
				{
					float x = f * (-deltaUV1.x * edge0.x + deltaUV0.x * edge1.x);
					float y = f * (-deltaUV1.x * edge0.y + deltaUV0.x * edge1.y);
					float z = f * (-deltaUV1.x * edge0.z + deltaUV0.x * edge1.z);
					
					Math::Vector3 binormal0 = Math::Vector3(x, y, z);
					
					binormal0.Normalize();
					
					v0.binormal[0] = binormal0.GetX();
					v0.binormal[1] = binormal0.GetY();
					v0.binormal[2] = binormal0.GetZ();
					
					v1.binormal[0] = binormal0.GetX();
					v1.binormal[1] = binormal0.GetY();
					v1.binormal[2] = binormal0.GetZ();
					
					v2.binormal[0] = binormal0.GetX();
					v2.binormal[1] = binormal0.GetY();
					v2.binormal[2] = binormal0.GetZ();
				}
			}
		}
		
#if 1
		{
			for(int32_t i = 0; i < sizeof(vertex) / sizeof(vertex[0]); i++)
			{
				VertexData& v = vertex[i];
				
				printf(">>> p:[%f][%f][%f] n:[%f][%f][%f] t:[%f][%f][%f] b:[%f][%f][%f] uv:[%f][%f]\n", v.position[0],v.position[1],v.position[2],v.normal[0],v.normal[1],v.normal[2],v.tangent[0],v.tangent[1],v.tangent[2],v.binormal[0],v.binormal[1],v.binormal[2],v.texcoord[0],v.texcoord[1]);
			}
		}
#endif
		
		// 頂点バッファの生成.
		{
			g_vertexBuff = g_device.NewBuffer(vertex, sizeof(vertex), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
			assert(g_vertexBuff);
			
			g_vertexNum = sizeof(vertex) / sizeof(vertex[0]);
			
			//printf(">>> vertex num : [%d]\n", g_vertexNum);
		}
	
		// インデックスバッファの生成.
		{
			g_indexBuff = g_device.NewBuffer(index, sizeof(index), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
			assert(g_indexBuff);
			
			g_indexNum = sizeof(index) / sizeof(index[0]);
			
			//printf(">>> index num : [%d]\n", g_indexNum);
		}
	}
	
	
	// レンダーパイプラインの生成.
	{
		mtlpp::RenderPipelineDescriptor desc;
		
		desc.SetVertexFunction(normalMappingVS);
		desc.SetFragmentFunction(normalMappingFS);
		
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
	
	
	// テクスチャの読み込み.
	{
		const std::string fileName = "data/brickwall.jpg";
		
		mtlpp::TextureLoader textureLoader(g_device);
		assert(textureLoader);
		
		mtlpp::Texture texture = textureLoader.NewTextureWithPath(fileName.c_str());
		assert(texture);
		
		g_diffuseMap = texture;
	}
	
	{
		const std::string fileName = "data/brickwall_normal.jpg";
		
		mtlpp::TextureLoader textureLoader(g_device);
		assert(textureLoader);
		
		mtlpp::Texture texture = textureLoader.NewTextureWithPath(fileName.c_str());
		assert(texture);
		
		g_normalMap = texture;
	}
	
	
	// カメラの初期化.
	{
		g_camera.SetScreenW(SCREEN_WIDTH);
		g_camera.SetScreenH(SCREEN_HEIGHT);
		g_camera.SetFovY(Math::ToRadians(60.0f));
		g_camera.SetNear(1.0f);
		g_camera.SetFar(10000.0f);
	}
	
	Window window(g_device, &doFrame, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	g_frameCount = Timer::GetCurrentTime();
	
	Window::Run();
	
	return 0;
}
