#pragma once

#include "stdafx.h"
#include "mercator_util.h"

typedef uint32_t temporal_t;
typedef uint8_t categorical_t;

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

   inline bool contains(const spatial_t& other) const {
      if (other.z > z) {
         uint32_t n = 1 << (other.z - z);

         uint32_t x_min = static_cast<uint32_t>(x) * n;
         uint32_t x_max = x_min + n;

         uint32_t y_min = static_cast<uint32_t>(y) * n;
         uint32_t y_max = y_min + n;

         return x_min <= other.x && x_max >= other.x && y_min <= other.y && y_max >= other.y;
      } else if (other.z == z) {
         return x == other.x && y == other.y;
      } else {
         return false;
      }
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

inline void toJSON(rapidjson::Writer<rapidjson::StringBuffer>& writer, const uint64_t& data) {
   writer.Uint(data);
}

inline void toJSON(rapidjson::Writer<rapidjson::StringBuffer>& writer, const spatial_t& data) {
   writer.Uint(data.x);
   writer.Uint(data.y);
   writer.Uint(data.z);
}

struct region_t {
   region_t() = default;

   region_t(uint32_t _x0, uint32_t _y0, uint32_t _x1, uint32_t _y1, uint8_t _z)
      : tile0(_x0, _y0, _z), tile1(_x1, _y1, _z), z(_z) { }

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

   inline bool cover(const spatial_t& tile) const {
      if (z > tile.z) {
         uint32_t n = 1 << (z - tile.z);

         uint32_t x_min = static_cast<uint32_t>(tile.x) * n;
         uint32_t x_max = x_min + n;

         uint32_t y_min = static_cast<uint32_t>(tile.y) * n;
         uint32_t y_max = y_min + n;

         return (x_min >= x0() && y_min >= y0() && x_max <= x1()  && y_max <= y1());
      } else return false;
   }

   inline bool intersect(const spatial_t& tile) const {
      if (z > tile.z) {
         uint32_t n = 1 << (z - tile.z);

         uint32_t x_min = static_cast<uint32_t>(tile.x) * n;
         uint32_t x_max = x_min + n;

         uint32_t y_min = static_cast<uint32_t>(tile.y) * n;
         uint32_t y_max = y_min + n;

         return (x0() <= x_max && y0() <= y_max && x1() >= x_min && y1() >= y_min);
      } else if (z == tile.z) {
         return x0() <= tile.x && y0() <= tile.y && x1() >= tile.x && y1() >= tile.y;
      } else {
         return false;
      }
   }

   friend std::ostream& operator<<(std::ostream& os, const region_t& obj) {
      return os << (uint32_t)obj.z << "/" << obj.x0() << "/" << obj.y0()
         << "/" << obj.x1() << "/" << obj.y1();
   }

   spatial_t tile0, tile1;
   uint8_t z;
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

   friend std::ostream& operator<<(std::ostream& os, const interval_t& obj) {
      return os << obj.bound[0] << "/" << obj.bound[1];
   }

   std::array<temporal_t, 2> bound;
};
