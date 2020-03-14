//
//  StringUtility.h
//  utility
//
//  Created by TAKEZOH on 2020/03/14.
//  Copyright © 2020 TAKEZOH. All rights reserved.
//

#pragma once

#include <string>
#include <vector>

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

