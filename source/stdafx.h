// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#define RAPIDJSON_SSE42

// Windows specific defines
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS

// C includes
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>

// C++ includes
#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <queue>
#include <regex>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>

// boost library
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// ResidentSetSize
#include <nadeau/ResidentSetSize.h>

// mongoose
#include <mongoose.h>

// rapidjson
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

// dynarray
#include <robbepop/dynarray.hpp>

// timsort
#include <gfx/timsort.hpp>

#include "types.h"