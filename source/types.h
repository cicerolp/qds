#pragma once

#include "stdafx.h"

typedef uint8_t binary_offset;

struct BinaryHeader {
   uint32_t bytes;
   uint32_t records;
};