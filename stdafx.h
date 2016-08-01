// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#undef NDEBUG
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS

// C includes
#include <cmath>
#include <cstdio>
#include <cassert>

// C++ includes
#include <set>
#include <map>
#include <deque>
#include <regex>
#include <array>
#include <tuple>
#include <thread>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <unordered_map>

// boost library
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// ResidentSetSize
#include <nadeau/ResidentSetSize.h>

// mongoose
#include <mongoose/mongoose.h>

// rapidjson
#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

// dynarray
#include <robbepop/dynarray.hpp>

#include "types.h"