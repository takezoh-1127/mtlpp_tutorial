//
//  Timer.h
//
//  Created by TAKEZOH on 2020/02/15.
//  Copyright © 2020 TAKEZOH. All rights reserved.
//

#pragma once

#include <chrono>

//namepace xe
//{


/**
	ゲーム内の時間計測で使用する高精度のタイマークラス.
	C++11のstd::chronoを使用して実装してみる.
 */
class Timer
{
public:
	// マルチプラットフォーム等で値の型を隠蔽するためにここで宣言しておく.
	//using ValueType = std::chrono::system_clock::time_point;
	using ValueType = uint64_t;

public:
	static void Initialize()
	{
	
	}
	
	/**
		現在の時間を取得.
	 */
	static const Timer GetCurrentTime()
	{
		Timer t;
		
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

		t.value_ = msec.count();
		
		return t;
	}

public:
	
	/**
	 */
	Timer()
	{
	}
	
	/**
	 */
	Timer(const Timer& t)
	{
		value_ = t.value_;
	}
	
	/**
		秒の取得.
	 */
	float GetSecond() const
	{
		return static_cast<float>(value_) / 1000.0f;
	}
	
	
	// 算術演算子.
	
	/**
		+ 演算子.
	 */
	friend const Timer operator + (const Timer& lhs, const Timer& rhs)
	{
		Timer t;
		t.value_ = lhs.value_ + rhs.value_;
		return t;
	}
	
	/**
		- 演算子.
	 */
	friend const Timer operator - (const Timer& lhs, const Timer& rhs)
	{
		Timer t;
		t.value_ = lhs.value_ - rhs.value_;
		return t;
	}
	
	/**
	 */
	friend const Timer operator % (const Timer& lhs, const Timer& rhs)
	{
		Timer t;
		t.value_ = lhs.value_ % rhs.value_;
		return t;
	}
	
	/**
	 */
	friend const Timer operator * (const Timer& lhs, float val)
	{
		Timer t;
		t.value_ = lhs.value_;
		t *= val;
		return t;
	}
	
	/**
	 */
	friend const Timer operator * (const Timer& lhs, uint32_t val)
	{
		Timer t;
		t.value_ = lhs.value_;
		t *= val;
		return t;
	}
	
	/**
	 */
	friend const Timer operator * (const Timer& lhs, int32_t val)
	{
		Timer t;
		t.value_ = lhs.value_;
		t *= val;
		return t;
	}
	
	/**
	 */
	friend const Timer operator / (const Timer& lhs, float val)
	{
		Timer t;
		t.value_ = lhs.value_;
		t /= val;
		return t;
	}
	
	/**
	 */
	friend const Timer operator / (const Timer& lhs, uint32_t val)
	{
		Timer t;
		t.value_ = lhs.value_;
		t /= val;
		return t;
	}
	
	/**
	 */
	friend const Timer operator / (const Timer lhs, uint32_t val)
	{
		Timer t;
		t.value_ = lhs.value_;
		t /= val;
		return t;
	}
	
	/**
	 */
	friend const Timer operator / (const Timer lhs, int32_t val)
	{
		Timer t;
		t.value_ = lhs.value_;
		t /= val;
		return t;
	}
	
	
	// 代入演算子.
	
	/**
	 */
	const Timer& operator = (const Timer& rhs)
	{
		value_ = rhs.value_;
		return *this;
	}
	
	/**
	 */
	const Timer& operator += (const Timer& rhs)
	{
		value_ += rhs.value_;
		return *this;
	}
	
	/**
	 */
	const Timer& operator -= (const Timer& rhs)
	{
		value_ -= rhs.value_;
		return *this;
	}
	
	/**
	 */
	const Timer& operator %= (const Timer& rhs)
	{
		value_ %= rhs.value_;
		return *this;
	}
	
	/**
	 */
	const Timer& operator *= (float val)
	{
		value_ = static_cast<ValueType>(static_cast<double>(value_) * static_cast<double>(val));
		return *this;
	}
	
	/**
	 */
	const Timer& operator *= (uint32_t val)
	{
		value_ = value_ * val;
		return *this;
	}
	
	/**
	 */
	const Timer& operator *= (int32_t val)
	{
		value_ = value_ * val;
		return *this;
	}
	
	/**
	 */
	const Timer& operator /= (float val)
	{
		value_ = static_cast<ValueType>(static_cast<double>(value_) / static_cast<double>(val));
		return *this;
	}
	
	/**
	 */
	const Timer& operator /= (uint32_t val)
	{
		value_ = value_ / val;
		return *this;
	}
	
	/**
	 */
	const Timer& operator /= (int32_t val)
	{
		value_ = value_ / val;
		return *this;
	}
	
	
	
	// 比較演算子.
	
	/**
	 */
	friend bool operator == (const Timer& lhs, const Timer& rhs)
	{
		return lhs.value_ == rhs.value_;
	}
	
	/**
	 */
	friend bool operator != (const Timer& lhs, const Timer& rhs)
	{
		return lhs.value_ != rhs.value_;
	}
	
	/**
	 */
	friend bool operator < (const Timer& lhs, const Timer& rhs)
	{
		return lhs.value_ < rhs.value_;
	}
	
	/**
	 */
	friend bool operator > (const Timer& lhs, const Timer& rhs)
	{
		return lhs.value_ > rhs.value_;
	}
	
	/**
	 */
	friend bool operator <= (const Timer& lhs, const Timer& rhs)
	{
		return lhs.value_ <= rhs.value_;
	}
	
	/**
	 */
	friend bool operator >= (const Timer& lhs, const Timer& rhs)
	{
		return lhs.value_ >= rhs.value_;
	}
	
private:
	ValueType value_;
	//std::chrono::system_clock::time_point value_;
};

//}
