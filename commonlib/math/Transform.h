//
//  Transform.h
//  tutorial_05
//
//  Created by TAKEZOH on 2019/11/28.
//  Copyright © 2019 TAKEZOH. All rights reserved.
//

#pragma once

#include "Math.h"

/**
 */
class Transform
{
public:
	/**
	 */
	void SetPosition(Math::Vector3 pos)
	{
		position_ = pos;
	}
	
	/**
	 */
	Math::Vector3 GetPosition() const
	{
		return position_;
	}
	
	/**
	 */
	void SetRotation(Math::Quaternion rot)
	{
		rotation_ = rot;
	}
	
	/**
	 */
	Math::Quaternion GetRotation() const
	{
		return rotation_;
	}
	
	/**
	 */
	void SetScale(Math::Vector3 scale)
	{
		scale_ = scale;
	}
	
	/**
	 */
	Math::Vector3 GetScale() const
	{
		return scale_;
	}
	
	/**
	 */
	Math::Matrix4 GetWorldMatrix() const
	{
		Math::Matrix4 mat;
		
		mat = Math::Matrix4::CreateScale(scale_);
		mat *= Math::Matrix4::CreateFromQuaternion(rotation_);
		mat *= Math::Matrix4::CreateTranslation(position_);
		
		return mat;
	}
private:
	Math::Vector3 position_ = { Math::Vector3::Zero() };
	Math::Quaternion rotation_ = { Math::Quaternion::Identity() };
	Math::Vector3 scale_ = { Math::Vector3::One() };
};

