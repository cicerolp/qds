#pragma once

#include "stdafx.h"
#include "mercator_util.h"

typedef uint8_t categorical_t;
typedef uint32_t temporal_t;

struct spatial_t {
   spatial_t() = default;

   spatial_t(uint32_t _x, uint32_t _y, uint8_t _z, uint8_t _l = 0)
      : x(_x), y(_y), z(_z), leaf(_l) {}

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
   uint64_t leaf : 1;
};

struct coordinates_t { float lat, lon; };

struct BinaryHeader {
   uint32_t bytes;
   uint32_t records;
};

struct tile_t : public spatial_t {
   tile_t() = default;

   tile_t(uint32_t x, uint32_t y, uint8_t z) :
      spatial_t(x, y, z) {
      lat0 = mercator_util::tiley2lat(y, z);
      lon0 = mercator_util::tilex2lon(x, z);

      lat1 = mercator_util::tiley2lat(y + 1, z);
      lon1 = mercator_util::tilex2lon(x + 1, z);
   }

   inline bool operator ==(const spatial_t& spatial) const {
      return z == spatial.z && x == spatial.x && y == spatial.y;
   }

   friend std::ostream& operator<< (std::ostream& stream, const tile_t& tile) {
      stream << tile.x << "/" << tile.y << "/" << tile.z;
      return stream;
   }

   float lat0, lon0, lat1, lon1;
};

struct interval_t {
   interval_t() = default;

   interval_t(temporal_t lower, temporal_t upper)
      : bound{ lower, upper } {};

   std::array<temporal_t, 2> bound;
};
