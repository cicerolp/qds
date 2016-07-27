#pragma once

#include "stdafx.h"
#include "mercator_util.h"

typedef uint8_t categorical_t;
typedef uint32_t temporal_t;

struct spatial_t {
   spatial_t() : spatial_t(0, 0, 0) { }

   spatial_t(uint32_t _x, uint32_t _y, uint8_t _z, uint8_t _l = 1)
      : x(_x), y(_y), z(_z), leaf(_l) {

      if (x % 2 == 0) {
         if (y % 2 == 0) index = 0;
         else index = 1;
      } else {
         if (y % 2 == 0) index = 2;
         else index = 3;
      }
   }

   inline bool operator ==(const spatial_t& other) const {
      return (z == other.z && x == other.x && y == other.y);
   }

   // index
   inline /*explicit*/ operator uint8_t() const {
      return index;
   }

   inline bool intersects(const spatial_t& other) const {
      return (z < other.z) ? contains(other) : other.contains(*this);
   }

   inline bool contains(const spatial_t& other) const {
      const uint32_t n = 2 << (other.z - z - 1);

      const uint32_t x_min = static_cast<uint32_t>(x) * n;
      const uint32_t x_max = x_min + n;

      const uint32_t y_min = static_cast<uint32_t>(y) * n;
      const uint32_t y_max = y_min + n;

      return x_min <= other.x && x_max >= other.x && y_min <= other.y && y_max >= other.y;
   }

   friend std::ostream& operator<<(std::ostream& stream, const spatial_t& tile) {
      stream << tile.x << "/" << tile.y << "/" << tile.z;
      return stream;
   }

   union {
      struct {
         uint64_t x : 25;
         uint64_t y : 25;
         uint64_t z : 5;
         uint64_t index : 2;
         uint64_t leaf : 1;
      };

      uint64_t data;
   };
};

struct region_t {
   region_t() = default;

   region_t(uint32_t _x0, uint32_t _y0, uint32_t _x1, uint32_t _y1, uint8_t _z)
      : tile0(_x0, _y0, _z), tile1(_x1, _y1, _z) { }

   inline uint32_t x0() const {
      return tile0.x;
   }

   inline uint32_t x1() const {
      return tile1.x;
   }

   inline uint32_t y0() const {
      return tile0.y;
   }

   inline uint32_t y1() const {
      return tile1.y;
   }

   inline uint32_t z() const {
      return tile0.z;
   }

   bool intersect(const spatial_t& tile) const {
      if (tile.z < z()) {
         //const uint32_t n = static_cast<uint32_t>(std::pow(2, z() - tile.z));
         const uint32_t n = 2 << (z() - tile.z - 1);

         const uint32_t x_min = static_cast<uint32_t>(tile.x) * n;
         const uint32_t x_max = x_min + n;

         const uint32_t y_min = static_cast<uint32_t>(tile.y) * n;
         const uint32_t y_max = y_min + n;

         return (x1() >= x_min && x0() <= x_max && y1() >= y_min && y0() <= y_max);

      } else if (z() == tile.z) {
         return x0() <= tile.x && x1() >= tile.x && y0() <= tile.y && y1() >= tile.y;

      } else {
         return false;
      }
   }

   spatial_t tile0, tile1;
};

struct coordinates_t { float lat, lon; };

struct BinaryHeader {
   uint32_t bytes;
   uint32_t records;
};

struct interval_t {
   interval_t() = default;

   interval_t(temporal_t lower, temporal_t upper)
      : bound{ lower, upper } {};

   inline bool contain(const temporal_t& lower, const temporal_t& upper) const {
      return bound[0] <= lower && bound[1] >= upper;
   }

   std::array<temporal_t, 2> bound;
};
