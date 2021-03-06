//
//  textureloader.hpp
//  test_mtlpp
//
//  Created by TAKEZOH on 2019/09/19.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

#pragma once

#ifndef textureloader_h
#define textureloader_h

#include "mtlpp.hpp"

namespace mtlpp
{

/** MTKTextureLoaderのラッパー.
	mtlppには用意されていないようなので自前で用意.
	namespaceは統一するためにmtlppにしておく.
	
	https://developer.apple.com/documentation/metalkit/mtktextureloader?language=objc
	
	PNG、JPEG、TIFFなどのフォーマットの読み込み.
	KTX、PVRファイル、アセットカタログ、Core Graphics画像、その他のソースから画像データを読み込むことも可能.
	画像データから出力テクスチャ形式とピクセル形式を推測.
	オプションを指定して、画像の読み込みとテクスチャの作成プロセスを変更できる.
*/
class TextureLoader : public ns::Object
{
public:
	TextureLoader(const Device device);
	
	Texture NewTextureWithPath(const ns::String& filePath, ns::Error* error = nullptr);
};

}

#endif /* textureloader_h */
