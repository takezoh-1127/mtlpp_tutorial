//
//  main.cpp
//  tutorial_09
//
//  Created by TAKEZOH on 2020/01/18.
//  Copyright © 2020 TAKEZOH. All rights reserved.
//

//
//	Wavefront OBJ(.obj)ファイルを読み込んで表示するサンプルプログラム.
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
int32_t g_vertexNum = 0;
int32_t g_indexNum = 0;

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
	Math::Vector3 cameraPos;
	Math::Vector3 diffuseColor;
	Math::Vector3 specularColor;
	Math::Vector3 ambientColor;
	float specularPower;
}UniformBuffer;


namespace Utility
{

/**
	文字列を指定セパレータ文字で分解.
*/
std::vector<std::string> SplitString(const std::string &str, char sep)
{
    std::vector<std::string> v;
    std::stringstream ss(str);
    std::string buffer;

    while( std::getline(ss, buffer, sep) )
	{
        v.emplace_back(buffer);
    }

    return v;
}

} // namespace Utility



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
		Math::Vector3 cameraPos = Math::Vector3(0.0f, 6.0f, -25.0f);
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
		
		//Math::Quaternion incY(Math::Vector3::UnitY(), angle * 0.5f);
		Math::Quaternion incY(Math::Vector3::UnitY(), Math::ToRadians(180.0f));
		//Math::Quaternion incX(Math::Vector3::UnitX(), angle * 0.2f);
		//Math::Quaternion incX(Math::Vector3::UnitX(), Math::ToRadians(45.0f*0.5f));
			
		auto rot = g_transform.GetRotation();
			
		rot = Math::Quaternion::Concatenate(rot, incY);
		//rot = Math::Quaternion::Concatenate(rot, incX);
		
		rot = incY;
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
		//encoder.SetCullMode(mtlpp::CullMode::Front);
					
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
	
	mtlpp::Function vertFunc = library.NewFunction("phongVS");
	assert(vertFunc);
	
	mtlpp::Function fragFunc = library.NewFunction("phongFS");
	assert(fragFunc);
	
	// テクスチャの読み込み.
	{
		mtlpp::TextureLoader textureLoader(g_device);
		assert(textureLoader);
		
		mtlpp::Texture texture = textureLoader.NewTextureWithPath("data/chr_knight/chr_knight.png");
		//mtlpp::Texture texture = textureLoader.NewTextureWithPath("data/chr_rain/chr_rain.png");
		//mtlpp::Texture texture = textureLoader.NewTextureWithPath("data/chr_sword/chr_sword.png");
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
	
	// objファイルの読み込み.
	// ここでとりえあえず左手系に変換しておく.
	
	std::vector<VertexData> vertexArray;
	std::vector<int32_t> indexArray;
	
	{
		std::string fileName = "data/chr_knight/chr_knight.obj";
		//std::string fileName = "data/chr_rain/chr_rain.obj";
		//std::string fileName = "data/chr_sword/chr_sword.obj";
		
		std::ifstream file;

		file.open(fileName.c_str(), std::ios::in);

		if(!file.is_open())
		{
			assert(false);
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
	}
	
	// 頂点バッファの生成.
	g_vertexNum = (int32_t)vertexArray.size();
	g_vertexBuff = g_device.NewBuffer(&vertexArray[0], sizeof(VertexData) * g_vertexNum, mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
	assert(g_vertexBuff);
	
	// インデックスバッファの生成.
	g_indexNum = (int32_t)indexArray.size();
	g_indexBuff = g_device.NewBuffer(&indexArray[0], sizeof(int32_t) * g_indexNum, mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
	assert(g_indexBuff);
	
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
	
	//
	Window window(g_device, &doFrame, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	Window::Run();
	
	return 0;
}
