#pragma once

#include "stdafx.h"
#include <boost/functional/hash.hpp>

#include "mercator_util.h"

struct spatial_t {
	spatial_t() = default;

	spatial_t(uint32_t _x, uint32_t _y, uint8_t _z, uint8_t _l = 0)
		: x(_x), y(_y), z(_z), l(_l) {}

	inline bool operator ==(const spatial_t& other) const {
		return (z == other.z && x == other.x && y == other.y);
	}
   inline operator uint32_t() const {
      return mercator_util::get_index(x, y);
   }

	friend std::ostream& operator<<(std::ostream& stream, const spatial_t& tile) {
		stream << tile.x << "/" << tile.y << "/" << tile.z;
		return stream;
	}

	uint64_t x : 28;
	uint64_t y : 28;
	uint64_t z : 5;
	uint64_t l : 1;
};

namespace std {
	template<>
	struct hash<spatial_t> {
		std::size_t operator()(const spatial_t tile) const {
			return (tile.x * ((2 << tile.z) - 1)) + tile.y;
		}
	};
}

typedef uint8_t categorical_t;
typedef uint32_t temporal_t;

struct coordinates_t { float lat, lon; };

struct BinaryHeader {
	uint32_t bytes;
	uint32_t records;
};
