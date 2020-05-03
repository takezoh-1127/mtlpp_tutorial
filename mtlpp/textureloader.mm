//
//  textureloader.mm
//  test_mtlpp
//
//  Created by TAKEZOH on 2019/09/20.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

//#import <Foundation/Foundation.h>
#import <MetalKit/MetalKit.h>

#include "mtlpp.hpp"
#include "textureloader.hpp"

namespace mtlpp
{

/**
*/
TextureLoader::TextureLoader(const Device device)
 : ns::Object(ns::Handle{ (__bridge void*)[[MTKTextureLoader alloc] initWithDevice:(__bridge id<MTLDevice>)device.GetPtr()]})
{
}

/**
*/
Texture TextureLoader::NewTextureWithPath(const ns::String& filePath, ns::Error* error)
{
	Validate();
	
	// 一部のフォーマットで読み込んだテクスチャのRGBがおかしくなるケースがあるのでSRGBをNOにしてみる.
	NSDictionary *textureLoaderOptions =
    @{
      MTKTextureLoaderOptionTextureUsage       : @(MTLTextureUsageShaderRead),
      MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModePrivate),
      MTKTextureLoaderOptionSRGB               : @(NO)
      };

	NSString* path = (__bridge NSString*)filePath.GetPtr();
	NSURL* url = [NSURL fileURLWithPath:path];
	
	// Error
	NSError* nsError = NULL;
	NSError** nsErrorPtr = error ? &nsError : nullptr;
	//NSError** nsErrorPtr = &nsError;
	
	id<MTLTexture> texture;
	
#if 0
	texture = [(__bridge MTKTextureLoader*)m_ptr newTextureWithName:path scaleFactor:1.0 bundle:nil options:textureLoaderOptions error:nsErrorPtr];
#endif
	
	texture = [(__bridge MTKTextureLoader*)m_ptr newTextureWithContentsOfURL:url options:textureLoaderOptions error:nsErrorPtr];
	
	return ns::Handle{ (__bridge void*)texture };
}

}
