//
//  GfxCamera.h
//  gfx
//
//  Created by TAKEZOH on 2020/03/30.
//  Copyright © 2020 TAKEZOH. All rights reserved.
//

#pragma once

#include "Math.h"


namespace Gfx
{

/**
	カメラ.
 */
class Camera
{
public:
	void SetScreenW(uint32_t w)
	{
		if(screenW_ == w)
		{
			return;
		}
		
		screenW_ = w;
		dirty_ = true;
	}
	
	uint32_t GetScreenW() const { return screenW_; }
	
	void SetScreenH(uint32_t h)
	{
		if(screenH_ == h)
		{
			return;
		}
		
		screenH_ = h;
		dirty_ = true;
	}
	
	uint32_t GetScreenH() const { return screenH_; }
	
	void SetFovY(float fovY)
	{
		if(fovY_ == fovY)
		{
			return;
		}
		
		fovY_ = fovY;
		dirty_ = true;
	}
	
	float GetFovY() const { return fovY_; }
	
	void SetNear(float near)
	{
		if(near_ == near)
		{
			return;
		}
		
		near_ = near;
		dirty_ = true;
	}
	
	float GetNear() const { return near_; }
	
	void SetFar(float far)
	{
		if(far_ == far)
		{
			return;
		}
		
		far_ = far;
		dirty_ = true;
	}
	
	const Math::Matrix4& GetProjection()
	{
		if(dirty_)
		{
			float w = static_cast<float>(screenW_);
			float h = static_cast<float>(screenH_);
			
			projection_ = Math::Matrix4::CreatePerspectiveFOV(fovY_, w, h, near_, far_);
			
			dirty_ = false;
		}
		
		return projection_;
	}
	
	void SetView(const Math::Matrix4 &view)
	{
		view_ = view;
		
		invView_ = view_;
		invView_.Invert();
	}
	
	const Math::Matrix4& GetView() const { return view_; }
	const Math::Matrix4& GetInvView() const { return invView_; }

private:
	uint32_t screenW_ = { 0 };
	uint32_t screenH_ = { 0 };
	float fovY_ = { 0.0f };
	float near_ = { 0.0f };
	float far_ = { 0.0f };
	
	Math::Matrix4 projection_ = { Math::Matrix4::Identity };
	Math::Matrix4 view_ = { Math::Matrix4::Identity };
	Math::Matrix4 invView_ = { Math::Matrix4::Identity };
	
	bool dirty_ = { false };
};

}
