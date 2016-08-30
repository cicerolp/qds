// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS

// C includes
#include <cmath>
#include <cstdio>
#include <cassert>
#include <cstdint>

// C++ includes
#include <map>
#include <regex>
#include <array>
#include <queue>
#include <tuple>
#include <thread>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_map>

// boost library
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// ResidentSetSize
#include <nadeau/ResidentSetSize.h>

// mongoose
#include <mongoose/mongoose.h>

// rapidjson
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

// dynarray
#include <robbepop/dynarray.hpp>

// timsort
#include <gfx/timsort.hpp>

#include "types.h"