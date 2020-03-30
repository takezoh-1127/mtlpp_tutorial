//
//  main.cpp
//  GaussianFilter
//
//  Created by TAKEZOH on 2020/03/17.
//  Copyright © 2020 TAKEZOH. All rights reserved.
//

//
//	ガウスフィルターのサンプルプログラム.
//
//	(X)通常のフレームバッファに描画.
//	通常のフレームバッファをテクスチャとした場合、シェーダーから読み込むことができない（というか設定方法が分からない）ので
//	別の連打イングバッファを用意.
//	それをテクスチャとして参照し、横方向ぼかし用のフーレムバッファに描画.
//	ぼかしたテクスチャを縦方向にぼかしつつ通常のフレームバッファに描き戻す.
//


//#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>


#include "mtlpp.hpp"
#include "textureloader.hpp"


#include "window.hpp"

#include "Timer.h"

#include "Math.h"
#include "Transform.h"
#include "StringUtility.h"

#include "GfxCamera.h"


constexpr uint32_t SCREEN_WIDTH = 640;
constexpr uint32_t SCREEN_HEIGHT = 480;


// ぼかしフィルターバッファサイズ.
// パフォーマンスを改善するために縮小バッファにするのもあり.
constexpr uint32_t FILTER_BUFFER_WIDTH = 640;
constexpr uint32_t FILTER_BUFFER_HEIGHT = 480;


mtlpp::Device g_device;

mtlpp::CommandQueue g_commandQueue;

mtlpp::RenderPipelineState g_renderPipelineState;
mtlpp::RenderPipelineState g_renderPipelineState2;
mtlpp::RenderPipelineState g_renderPipelineState3;
mtlpp::DepthStencilState g_depthState;

mtlpp::VertexDescriptor g_vertex2DDesc;
mtlpp::VertexDescriptor g_vertexDesc;

mtlpp::Texture g_textureRenderBuff;			// テクスチャレンダリングバッファ.
mtlpp::Texture g_textureRenderDepthBuff;	// テクスチャレンダリング用のデプスバッファ.

mtlpp::Texture g_filterRenderBuff;			// 横方向ぼかしレンダリングバッファ.

mtlpp::Buffer g_filterVertexBuff;
mtlpp::Buffer g_filterIndexBuff;



// obj
mtlpp::Texture g_texture;

mtlpp::Buffer g_vertexBuff;
mtlpp::Buffer g_indexBuff;
int32_t g_vertexNum = 0;
int32_t g_indexNum = 0;

mtlpp::Buffer g_uniformBuff;

mtlpp::Buffer g_uniformFSBuff;

Timer g_frameCount;


Transform g_transform;


float g_filterTimer = 0.0f;

Gfx::Camera g_camera;


// 頂点データ.
typedef struct
{
	float position[3];
	float texcoord[2];
} VertexData2D;

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
	Math::Vector3 cameraPos;
	Math::Vector3 diffuseColor;
	Math::Vector3 specularColor;
	Math::Vector3 ambientColor;
	float specularPower;
}UniformBuffer;


// 頂点シェーダーに渡すユニフォームバッファ.
typedef struct
{
	float t;
	float w[8];
}UniformFSBuffer;


#if 0
namespace Gfx
{


enum class VertexFormat : uint32_t
{
	Position = 0x01 << 0,
	Normal = 0x01 << 1,
	Texcoord = 0x01 << 2,
};

class Mesh
{
public:
	Mesh(){}
	virtual ~Mesh(){}

private:
	void* vertexBuffer = { nullptr };
	
};

/**
 */
class ModelData
{
public:
	ModelData(){}
	virtual ~ModelData(){}
	
	bool Load(const std::string fileName){ return false; }

private:
	std::vector<Gfx::Mesh*> mesh_;
};

}
#endif



namespace OBJ
{

/**
	
 */
bool ReadFile(const std::string fileName,std::vector<VertexData> &vertexArray, std::vector<int32_t> &indexArray)
{
	std::ifstream file;

	file.open(fileName.c_str(), std::ios::in);

	if(!file.is_open())
	{
		assert(false);
		return false;
	}
		
	//std::array<float, 3>	vertex;
	std::vector<std::array<float, 3>> vertex;
	std::vector<std::array<float, 3>> normal;
	std::vector<std::array<float, 2>> texcoord;
		
	std::vector<std::array<int32_t, 3>> vertexIndex;
		
	//int32_t vertexCount = 0;
	//int32_t indexCount = 0;
		
	while(!file.eof())
	{
		std::string buff;

		std::getline(file, buff);

		//printf("  [%s]\n",buff.c_str());
			
		std::vector<std::string> vitem;
			
		vitem = Utility::SplitString(buff, ' ');
			
		if(vitem.size() == 0)
		{
			continue;
		}
			
		if(vitem[0] == "vn")
		{
			// 法線.
			float x = std::stof(vitem[1]);
			float y = std::stof(vitem[2]);
			float z = std::stof(vitem[3]);
				
			std::array<float, 3> n;
				
			n[0] = -x;
			n[1] = y;
			n[2] = z;
				
			normal.emplace_back(n);
		
		}
		else if(vitem[0] == "vt")
		{
			// texcoords.
			float u = std::stof(vitem[1]);
			float v = std::stof(vitem[2]);
				
			std::array<float, 2> uv;
				
			uv[0] = u;
			uv[1] = v;
				
			texcoord.emplace_back(uv);
		}
		else if(vitem[0] == "v")
		{
			// 頂点.
			float x = std::stof(vitem[1]);
			float y = std::stof(vitem[2]);
			float z = std::stof(vitem[3]);
				
			std::array<float, 3> v;
				
			v[0] = -x;
			v[1] = y;
			v[2] = z;
				
			//printf(" >>> v : [%f][%f][%f]\n", x, y, z);
				
			vertex.emplace_back(v);
		}
		else if(vitem[0] == "f")
		{
			// face.
			if(vitem.size() == 4)
			{
				// 三角ポリゴン.
				for(int32_t i = 0; i < 3; i++)
				{
					// 左手系にするためにCCWをCWになるように頂点のインデックスを入れ替え.
					int32_t CCWtoCW[] = { 0, 2, 1 };
						
					std::vector<std::string> vindex;
						
					vindex = Utility::SplitString(vitem[CCWtoCW[i] + 1], '/');
					//vindex = Utility::SplitString(vitem[i + 1], '/');
						
					if(vindex.size() != 3)
					{
						assert(false);
						continue;
					}
						
					int32_t index0 = std::stoi(vindex[0]) - 1;	// 頂点.
					int32_t index1 = std::stoi(vindex[1]) - 1;	// UV.
					int32_t index2 = std::stoi(vindex[2]) - 1;	// 法線.
					
					assert(index0 >= 0 && index0 < vertex.size());
					assert(index1 >= 0 && index1 < texcoord.size());
					assert(index2 >= 0 && index2 < normal.size());
						
						
					std::array<int32_t, 3> index;
						
					index[0] = index0;
					index[1] = index1;
					index[2] = index2;
						
					// 同じインデックスの組み合わせの重複チェック.
					{
						bool check = false;
							
						int32_t ii = (int32_t)vertexIndex.size();
							
						//for(auto& vi : vertexIndex)
						for(int32_t i = 0; i < vertexIndex.size(); i++)
						{
							auto& vi = vertexIndex[i];
								
							if(vi[0] == index[0] && vi[1] == index[1] && vi[2] == index[2])
							{
								//
								//printf(" >>> find same index. [%d][%d][%d]\n", index[0], index[1], index[2]);
								ii = i;
								check = true;
								break;
							}
						}
							
						indexArray.emplace_back(ii);
							
						if(!check)
						{
							//printf(" +++ add index. [%d][%d][%d]\n", index[0], index[1], index[2]);
							vertexIndex.emplace_back(index);
								
							VertexData vertexData;
								
							std::array<float, 3> v;
							std::array<float, 2> uv;
							std::array<float, 3> n;
								
							v = vertex[index0];
							uv = texcoord[index1];
							n = normal[index2];
								
							vertexData.position[0] = v[0];
							vertexData.position[1] = v[1];
							vertexData.position[2] = v[2];
								
							vertexData.normal[0] = n[0];
							vertexData.normal[1] = n[1];
							vertexData.normal[2] = n[2];
								
							vertexData.texcoord[0] = uv[0];
							vertexData.texcoord[1] = uv[1];
								
							vertexArray.emplace_back(vertexData);
						}
					}
						
					//indexCount++;
					
					//indexArray.
				}
			}
			else if(vitem.size() == 5)
			{
				// 四角ポリゴン.
				assert(false);
			}
			else
			{
				// それ以外は非対応.
				assert(false);
			}
		}
		
	}
		
	//printf("### index count : [%d][%d]\n", indexCount, (int32_t)vertexIndex.size());
	//printf("### vertex count : [%d]\n", (int32_t)vertex.size());
	//printf("### vertex array count : [%d]\n", (int32_t)vertexArray.size());
	//printf("### index array count : [%d]\n", (int32_t)indexArray.size());
		
	//for(auto index : indexArray)
	//{
		//printf(" >>> index : [%d]\n", index);
	//}
	
	file.close();
	
	return true;
}

}


/**
 */
void update(float deltaT)
{
	// 通常シーンのビューポート.
	{
		//Math::Vector3 cameraPos = Math::Vector3(0.0f, 0.0f, -4.0f);
		Math::Vector3 cameraPos = Math::Vector3(0.0f, 7.0f, -25.0f);
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
		//Math::Quaternion incX(Math::Vector3::UnitX(), angle * 0.2f);
		//Math::Quaternion incX(Math::Vector3::UnitX(), Math::ToRadians(45.0f*0.5f));
			
		auto rot = g_transform.GetRotation();
			
		rot = Math::Quaternion::Concatenate(rot, incY);
		//rot = Math::Quaternion::Concatenate(rot, incX);
		
		//rot = incY;
		//rot = incX;
			
		g_transform.SetRotation(rot);
			
		//g_transform[0].SetPosition(Math::Vector3(2.0f,0.0f,0.0f));
	}
	
	{
		g_filterTimer += deltaT * 0.3f;
		g_filterTimer = g_filterTimer > 1.5f ? 0.0f : g_filterTimer;
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
			
		// 視点をローカル座標系に変換.
		{
			auto WV = g_transform.GetWorldMatrix() * view;
				
			WV.Invert();
				
			buffer->cameraPos = WV.GetTranslation();
		}
			
		// ライトの向きをローカル座標系に変換.
		{
			auto lightDir = Math::Vector3::Normalize(Math::Vector3(0.7f, -0.7f, 0.7f));
					
			auto invLW = buffer->invLW;
					
			//buffer->lightDir = Math::Vector3::Normalize(Math::Vector3(0.7f, -0.7f, 0.7f));
			buffer->lightDir = Math::Vector3::Transform(lightDir, invLW, 0.0f);
		}
				
		// 光源色を頂点シェーダーに渡す.
		{
			buffer->ambientColor = Math::Vector3(0.2f, 0.2f, 0.2f);
			//buffer->ambientColor = Math::Vector3(0.0f, 0.0f, 0.0f);
				
			buffer->diffuseColor = Math::Vector3(1.0f, 1.0f, 1.0f);
			buffer->specularColor = Math::Vector3(0.2f, 0.2f, 0.2f);
					
			buffer->specularPower = 40.0f;
		}
	}
	
	// ユニフォームバッファの設定.
	{
		UniformFSBuffer* buffer = static_cast<UniformFSBuffer*>(g_uniformFSBuff.GetContents());
		
		buffer->t = Math::Clamp(g_filterTimer, 0.0f, 1.0f);
		
		// ウェイト値の計算.
		float total = 0.0f;
		std::array<float, 8> w = { 0.0f };
		float d = 20.0f;
		
		for(int32_t i = 0; i < w.size(); i++)
		{
			float pos = 1.0f + (2.0f * static_cast<float>(i));
			
			w[i] = ::expf(-0.5 *(pos*pos) / d);
			total += w[i];
		}
		
		// 正規化.
		// シェーダー側で２ピクセル分足し込んでいくのでここでは足して0.5になるように正規化する.
		for(int32_t i = 0; i < w.size(); i++)
		{
			w[i] /= total * 2.0f;
		}
		
		for(int32_t i = 0; i < w.size(); i++)
		{
			buffer->w[i] = w[i];
		}
	}

}

/**
 */
void render(const Window& window)
{
	// 通常シーンの描画.
	{
		mtlpp::CommandBuffer commandBuffer = g_commandQueue.CommandBuffer();
		assert(commandBuffer);
		
		commandBuffer.SetLabel("Main command buffer");
		
		//mtlpp::RenderPassDescriptor desc = window.GetRenderPassDescriptor();
		mtlpp::RenderPassDescriptor desc;
			
		// テクスチャレンダリング用のレンダーパスの生成.
		{
			// フレームバッファの設定.
			{
				auto attach = desc.GetColorAttachments();
				
				attach[0].SetClearColor(mtlpp::ClearColor(0.2, 0.2, 0.8, 1.0));
				attach[0].SetLoadAction(mtlpp::LoadAction::Clear);
					
				attach[0].SetTexture(g_textureRenderBuff);
			}
				
			// デプスバッファの設定.
			{
				auto attach = desc.GetDepthAttachment();
					
				attach.SetClearDepth(1.0);
				attach.SetLoadAction(mtlpp::LoadAction::Clear);
					
				attach.SetTexture(g_textureRenderDepthBuff);
			}
				
			desc.SetRenderTargetArrayLength(1);
		}

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
			//encoder.SetCullMode(mtlpp::CullMode::Front);
				
			//encoder.SetFragmentSamplerStates();
				
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
				encoder.DrawIndexed(mtlpp::PrimitiveType::Triangle, g_indexNum, mtlpp::IndexType::UInt32, g_indexBuff, 0);
			}
						
			encoder.EndEncoding();
			
			//commandBuffer.Present(window.GetDrawable());
		}
		
		commandBuffer.Commit();
		//commandBuffer.WaitUntilCompleted();
	}
		
#if 1
	// 横方向のぼかしの描画.
	{
		mtlpp::CommandBuffer commandBuffer = g_commandQueue.CommandBuffer();
		assert(commandBuffer);
		
		commandBuffer.SetLabel("gaussian pass1 command buffer");
			
		// 縦方向ぼかし用のレンダーパスの生成.
		mtlpp::RenderPassDescriptor desc;
			
		{
			// フレームバッファの設定.
			{
				auto attach = desc.GetColorAttachments();
				
				attach[0].SetClearColor(mtlpp::ClearColor(0.0, 0.0, 0.0, 1.0));
				attach[0].SetLoadAction(mtlpp::LoadAction::Clear);
					
				attach[0].SetTexture(g_filterRenderBuff);
			}
				
#if 0
			// デプスバッファの設定.
			{
				auto attach = desc.GetDepthAttachment();
					
				attach.SetClearDepth(1.0);
				attach.SetLoadAction(mtlpp::LoadAction::Clear);
					
				attach.SetTexture(g_textureRenderDepthBuff);
			}
#endif
				
			desc.SetRenderTargetArrayLength(1);
		}
			
		if(desc)
		{
			mtlpp::RenderCommandEncoder encoder = commandBuffer.RenderCommandEncoder(desc);
			assert(encoder);
				
			// 面の向き.
			// 左手系はCW（時計周り）が表.
			encoder.SetFrontFacingWinding(mtlpp::Winding::Clockwise);
							
			// カリングモード.
			encoder.SetCullMode(mtlpp::CullMode::Back);
				
			encoder.SetRenderPipelineState(g_renderPipelineState2);
				
			encoder.SetFragmentBuffer(g_uniformFSBuff, 0, 0);
				
			encoder.SetVertexBuffer(g_filterVertexBuff, 0, 0);
			encoder.SetVertexBuffer(g_filterVertexBuff, 0, 1);
				
#if 0
			mtlpp::Texture renderTarget;
				
			{
				mtlpp::RenderPassDescriptor passDesc = window.GetRenderPassDescriptor();
				
				auto attach = passDesc.GetColorAttachments();
					
				renderTarget = attach[0].GetTexture();
					
				//renderTarget.
			}
				
			encoder.SetFragmentTexture(renderTarget, 0);
#endif
			//encoder.SetVertexTexture(g_textureRenderBuff, 0);
			encoder.SetFragmentTexture(g_textureRenderBuff, 0);
				
			encoder.DrawIndexed(mtlpp::PrimitiveType::TriangleStrip, 4, mtlpp::IndexType::UInt32, g_filterIndexBuff, 0);
			//encoder.EndEncoding();
			
			encoder.EndEncoding();
		}
			
		commandBuffer.Commit();
	}
#endif
		
	// 縦方向のぼかしの描画.
	// 通常のシーンに描き戻す.
	{
		mtlpp::CommandBuffer commandBuffer = g_commandQueue.CommandBuffer();
		assert(commandBuffer);
			
		commandBuffer.SetLabel("gaussian pass2 command buffer");
				
		mtlpp::RenderPassDescriptor desc = window.GetRenderPassDescriptor();
				
		auto attach = desc.GetColorAttachments();
		auto renderTarget = attach[0].GetTexture();
				
#if 0
		// 縦方向ぼかし用のレンダーパスの生成.
				
		mtlpp::RenderPassDescriptor desc;
				
		{
			// フレームバッファの設定.
			{
				auto attach = desc.GetColorAttachments();
					
				attach[0].SetClearColor(mtlpp::ClearColor(0.0, 0.0, 0.0, 1.0));
				attach[0].SetLoadAction(mtlpp::LoadAction::Clear);
						
				attach[0].SetTexture(g_filterRenderBuff);
			}
					
#if 0
			// デプスバッファの設定.
			{
				auto attach = desc.GetDepthAttachment();
						
				attach.SetClearDepth(1.0);
				attach.SetLoadAction(mtlpp::LoadAction::Clear);
						
				attach.SetTexture(g_textureRenderDepthBuff);
			}
#endif
					
			desc.SetRenderTargetArrayLength(1);
		}
#endif
				
		if(desc)
		{
			mtlpp::RenderCommandEncoder encoder = commandBuffer.RenderCommandEncoder(desc);
			assert(encoder);
					
			encoder.SetDepthStencilState(g_depthState);
				
			// 面の向き.
			// 左手系はCW（時計周り）が表.
			encoder.SetFrontFacingWinding(mtlpp::Winding::Clockwise);
								
			// カリングモード.
			encoder.SetCullMode(mtlpp::CullMode::Back);
					
			encoder.SetRenderPipelineState(g_renderPipelineState3);
				
			encoder.SetFragmentBuffer(g_uniformFSBuff, 0, 0);
				
			encoder.SetVertexBuffer(g_filterVertexBuff, 0, 0);
			encoder.SetVertexBuffer(g_filterVertexBuff, 0, 1);
					
#if 0
			mtlpp::Texture renderTarget;
					
			{
				mtlpp::RenderPassDescriptor passDesc = window.GetRenderPassDescriptor();
				
				auto attach = passDesc.GetColorAttachments();
						
				renderTarget = attach[0].GetTexture();
			}
#endif
					
			encoder.SetFragmentTexture(g_filterRenderBuff, 0);
					
			encoder.DrawIndexed(mtlpp::PrimitiveType::TriangleStrip, 4, mtlpp::IndexType::UInt32, g_filterIndexBuff, 0);
			//encoder.EndEncoding();
			
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



int main(int argc, const char * argv[])
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
	
	mtlpp::Function phongVS = library.NewFunction("phongVS");
	assert(phongVS);
	
	mtlpp::Function phongFS = library.NewFunction("phongFS");
	assert(phongFS);
	
	mtlpp::Function gaussianVS_pass1 = library.NewFunction("gaussianVS_pass1");
	assert(gaussianVS_pass1);
	
	mtlpp::Function gaussianFS_pass1 = library.NewFunction("gaussianFS_pass1");
	assert(gaussianFS_pass1);
	
	mtlpp::Function gaussianVS_pass2 = library.NewFunction("gaussianVS_pass2");
	assert(gaussianVS_pass2);
	
	mtlpp::Function gaussianFS_pass2 = library.NewFunction("gaussianFS_pass2");
	assert(gaussianFS_pass2);
	
	// 2D描画用の頂点バッファの定義.
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
		
		// texcoord.
		{
			auto attr = attributes[1];
			
			// u,v
			attr.SetFormat(mtlpp::VertexFormat::Float2);
			attr.SetOffset(12);
			attr.SetBufferIndex(1);
		}
		
		// position.
		{
			auto layout = layouts[0];
			
			layout.SetStride(20);	// 頂点バッファのサイズ. float(4byte) * 3 + float(4byte) * 3 + float(4byte) * 2
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}
		
		// texcoord.
		{
			auto layout = layouts[1];
			
			layout.SetStride(20);
			layout.SetStepRate(1);
			layout.SetStepFunction(mtlpp::VertexStepFunction::PerVertex);
		}
		
		g_vertex2DDesc = desc;
	}
	
	// モデル描画用の頂点バッファの定義.
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
	
	
	// テクスチャレンダリング用のフレームバッファ（テクスチャ）の生成.
	{
		mtlpp::TextureDescriptor desc;
		
		//MTLPixelFormatBGRA8Unorm_sRGB
		desc = mtlpp::TextureDescriptor::Texture2DDescriptor(mtlpp::PixelFormat::BGRA8Unorm, SCREEN_WIDTH, SCREEN_HEIGHT, false);
		
		// Private : GPUのみがアクセスできるように.
		desc.SetStorageMode(mtlpp::StorageMode::Private);
		
		// RenderTarget : レンダーパスでテクスチャにレンダリングするためのオプション.
		// ShaderRead : シェーダーのテクスチャから読み取りまたはサンプリングするためのオプション.
		uint32_t usage = (uint32_t)mtlpp::TextureUsage::RenderTarget | (uint32_t)mtlpp::TextureUsage::ShaderRead;
		desc.SetUsage(static_cast<mtlpp::TextureUsage>(usage));
		
		g_textureRenderBuff = g_device.NewTexture(desc);
		assert(g_textureRenderBuff);
	}

	// テクスチャレンダリング用のデプスバッファの生成.
	{
		mtlpp::TextureDescriptor desc;
		
		//MTLPixelFormatDepth32Float
		desc = mtlpp::TextureDescriptor::Texture2DDescriptor(mtlpp::PixelFormat::Depth32Float, SCREEN_WIDTH, SCREEN_HEIGHT, false);
		
		desc.SetStorageMode(mtlpp::StorageMode::Private);
		desc.SetUsage(mtlpp::TextureUsage::RenderTarget);
		
		g_textureRenderDepthBuff = g_device.NewTexture(desc);
		assert(g_textureRenderDepthBuff);
	}
	
	// 横方向ぼかし用のフレームバッファの生成.
	{
		mtlpp::TextureDescriptor desc;
		
		//MTLPixelFormatBGRA8Unorm_sRGB
		desc = mtlpp::TextureDescriptor::Texture2DDescriptor(mtlpp::PixelFormat::BGRA8Unorm, FILTER_BUFFER_WIDTH, FILTER_BUFFER_HEIGHT, false);
		
		// Private : GPUのみがアクセスできるように.
		desc.SetStorageMode(mtlpp::StorageMode::Private);
		
		// RenderTarget : レンダーパスでテクスチャにレンダリングするためのオプション.
		// ShaderRead : シェーダーのテクスチャから読み取りまたはサンプリングするためのオプション.
		uint32_t usage = (uint32_t)mtlpp::TextureUsage::RenderTarget | (uint32_t)mtlpp::TextureUsage::ShaderRead;
		desc.SetUsage(static_cast<mtlpp::TextureUsage>(usage));
		
		g_filterRenderBuff = g_device.NewTexture(desc);
		assert(g_filterRenderBuff);
	}
	
	// 横ぼかし用のレンダーパイプラインの生成.
	{
		mtlpp::RenderPipelineDescriptor desc;
			
		desc.SetVertexFunction(gaussianVS_pass1);
		desc.SetFragmentFunction(gaussianFS_pass1);
			
		desc.SetVertexDescriptor(g_vertex2DDesc);
			
		auto attach = desc.GetColorAttachments();
		attach[0].SetPixelFormat(mtlpp::PixelFormat::BGRA8Unorm);
		
#if 0
		attach[0].SetBlendingEnabled(true);
		
		//attach[0].SetSourceRgbBlendFactor(mtlpp::BlendFactor::Zero);
		attach[0].SetSourceRgbBlendFactor(mtlpp::BlendFactor::One);
		//attach[0].SetSourceRgbBlendFactor(mtlpp::BlendFactor::SourceColor);
		
		//attach[0].SetDestinationRgbBlendFactor(mtlpp::BlendFactor::One);
		//attach[0].SetDestinationRgbBlendFactor(mtlpp::BlendFactor::OneMinusSourceColor);
		//attach[0].SetDestinationRgbBlendFactor(mtlpp::BlendFactor::OneMinusDestinationColor);
		//attach[0].SetDestinationRgbBlendFactor(mtlpp::BlendFactor::BlendColor);
		attach[0].SetDestinationRgbBlendFactor(mtlpp::BlendFactor::Zero);
		
		//DestinationColor
		//OneMinusDestinationColor                            = 7,
        //DestinationAlpha                                    = 8,
        //OneMinusDestinationAlpha                            = 9,
        //SourceAlphaSaturated                                = 10,
        //BlendColor                                          = 11,
        //OneMinusBlendColor
		attach[0].SetRgbBlendOperation(mtlpp::BlendOperation::Add);
		//attach[0].SetRgbBlendOperation(mtlpp::BlendOperation::Subtract);
		//attach[0].SetRgbBlendOperation(mtlpp::BlendOperation::ReverseSubtract);
		//attach[0].SetRgbBlendOperation(mtlpp::BlendOperation::Min);
		//attach[0].SetRgbBlendOperation(mtlpp::BlendOperation::Max);
#endif
		
		g_renderPipelineState2 = g_device.NewRenderPipelineState(desc, nullptr);
		assert(g_renderPipelineState2);
	}
	
	// 縦ぼかし用のレンダーパイプラインの生成.
	{
		mtlpp::RenderPipelineDescriptor desc;
			
		desc.SetVertexFunction(gaussianVS_pass2);
		desc.SetFragmentFunction(gaussianFS_pass2);
			
		desc.SetVertexDescriptor(g_vertex2DDesc);
			
		auto attach = desc.GetColorAttachments();
		attach[0].SetPixelFormat(mtlpp::PixelFormat::BGRA8Unorm);
			
		desc.SetDepthAttachmentPixelFormat(mtlpp::PixelFormat::Depth32Float_Stencil8);
		desc.SetStencilAttachmentPixelFormat(mtlpp::PixelFormat::Depth32Float_Stencil8);
		
#if 0
		attach[0].SetBlendingEnabled(false);
		//attach[0].SetSourceRgbBlendFactor(mtlpp::BlendFactor::Zero);
		attach[0].SetSourceRgbBlendFactor(mtlpp::BlendFactor::SourceColor);
		
		//attach[0].SetDestinationRgbBlendFactor(mtlpp::BlendFactor::One);
		//attach[0].SetDestinationRgbBlendFactor(mtlpp::BlendFactor::OneMinusSourceColor);
		//attach[0].SetDestinationRgbBlendFactor(mtlpp::BlendFactor::OneMinusDestinationColor);
		attach[0].SetDestinationRgbBlendFactor(mtlpp::BlendFactor::BlendColor);
		
		attach[0].SetRgbBlendOperation(mtlpp::BlendOperation::Add);
#endif
		
		g_renderPipelineState3 = g_device.NewRenderPipelineState(desc, nullptr);
		assert(g_renderPipelineState3);
	}
	
	// モデル描画用のレンダーパイプラインの生成.
	// 「MetalProgramming Guide」によるとMTLRenderPipelineStateの生成は高コストなのでできるだけ再利用するように.
	// 毎フレームの生成などはしないようにした方が良いらしい...
	{
		mtlpp::RenderPipelineDescriptor desc;
		
		desc.SetVertexFunction(phongVS);
		desc.SetFragmentFunction(phongFS);
		
		// インデックスバッファの場合は頂点ディスクリプタを設定する必要がある？.
		desc.SetVertexDescriptor(g_vertexDesc);
		
		auto attach = desc.GetColorAttachments();
		attach[0].SetPixelFormat(mtlpp::PixelFormat::BGRA8Unorm);
		
		desc.SetDepthAttachmentPixelFormat(mtlpp::PixelFormat::Depth32Float);
		//desc.SetDepthAttachmentPixelFormat(mtlpp::PixelFormat::Depth32Float_Stencil8);
		//desc.SetStencilAttachmentPixelFormat(mtlpp::PixelFormat::Depth32Float_Stencil8);
		
		g_renderPipelineState = g_device.NewRenderPipelineState(desc, nullptr);
		assert(g_renderPipelineState);
	}
	
	// デプスステートの生成.
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
	
	// ユニフォームバッファの生成.
	{
		g_uniformFSBuff = g_device.NewBuffer(sizeof(UniformFSBuffer), mtlpp::ResourceOptions::StorageModeShared);
		assert(g_uniformFSBuff);
	}
	
	// モデルの読み込み.
	{
		std::string fileName = "data/chr_rain/chr_rain.obj";
		
		std::vector<VertexData> vertexArray;
		std::vector<int32_t> indexArray;
		
		OBJ::ReadFile(fileName, vertexArray, indexArray);
		
		// 頂点バッファの生成.
		g_vertexNum = (int32_t)vertexArray.size();
		g_vertexBuff = g_device.NewBuffer(&vertexArray[0], sizeof(VertexData) * g_vertexNum, mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
		assert(g_vertexBuff);
		
		// インデックスバッファの生成.
		g_indexNum = (int32_t)indexArray.size();
		g_indexBuff = g_device.NewBuffer(&indexArray[0], sizeof(int32_t) * g_indexNum, mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
		assert(g_indexBuff);
	}
	
	// テクスチャの読み込み.
	{
		const std::string fileName = "data/chr_rain/chr_rain.png";
		
		mtlpp::TextureLoader textureLoader(g_device);
		assert(textureLoader);
		
		mtlpp::Texture texture = textureLoader.NewTextureWithPath(fileName.c_str());
		assert(texture);
		
		g_texture = texture;
	}
	
	// フィルター用の頂点バッファの生成.
	// uv
	//	左上 : 0, 0
	//	左下 : 0, 1
	//	右上 : 1, 0
	//	右下 : 1, 1
	{
		VertexData2D v[] =
		{
			{ {-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f} },		// 左上.
			{ {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f} },		// 右上.
			{ {-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },		// 左下.
			{ {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f} },		// 右下.
		};
				
		g_filterVertexBuff = g_device.NewBuffer(v, sizeof(v), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
		assert(g_filterVertexBuff);
	}
		
	// インデックスバッファの生成.
	{
		uint32_t index[] =
		{
			0, 1, 2, 3,
		};
				
		g_filterIndexBuff = g_device.NewBuffer(index, sizeof(index), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
		assert(g_filterIndexBuff);
	}
	
	// カメラの初期化.
	{
		g_camera.SetScreenW(SCREEN_WIDTH);
		g_camera.SetScreenH(SCREEN_HEIGHT);
		g_camera.SetFovY(Math::ToRadians(60.0f));
		g_camera.SetNear(1.0f);
		g_camera.SetFar(10000.0f);
	}
	
	//
	Window window(g_device, &doFrame, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	g_frameCount = Timer::GetCurrentTime();
	
	Window::Run();
	
	return 0;
}
