#pragma once

#include "stdafx.h"
#include "mercator_util.h"

typedef uint8_t categorical_t;
typedef uint32_t temporal_t;

struct spatial_t {
   spatial_t(uint32_t _x, uint32_t _y, uint8_t _z, uint8_t _l = 0)
      : x(_x), y(_y), z(_z), leaf(_l) {
      index = mercator_util::get_index(x, y);
   }

   inline bool operator ==(const spatial_t& other) const {
      return (z == other.z && x == other.x && y == other.y);
   }

   // index
   inline /*explicit*/ operator uint8_t() const {
      return index;
   }  

   friend std::ostream& operator<<(std::ostream& stream, const spatial_t& tile) {
      stream << tile.x << "/" << tile.y << "/" << tile.z;
      return stream;
   }

   union {
      struct {
         uint64_t x     : 25;
         uint64_t y     : 25;
         uint64_t z     : 5;
         uint64_t index : 2;
         uint64_t leaf  : 1;         
      };
      uint64_t data;
   };
};

struct coordinates_t { float lat, lon; };

struct BinaryHeader {
   uint32_t bytes;
   uint32_t records;
};

struct tile_t : public spatial_t {
   tile_t() : spatial_t(0, 0, 0) { }

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
