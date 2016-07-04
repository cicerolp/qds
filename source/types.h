#pragma once

#include "stdafx.h"

typedef uint8_t categorical_t;

struct BinaryHeader {
   uint32_t bytes;
   uint32_t records;
};