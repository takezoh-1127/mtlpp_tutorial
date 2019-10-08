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
*/
class TextureLoader : public ns::Object
{
public:
	TextureLoader(const Device device);
	
	Texture NewTextureWithPath(const ns::String& filePath, ns::Error* error = nullptr);
};

}

#endif /* textureloader_h */
