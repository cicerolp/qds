#pragma once

#include "mercator_util.h"
#include "stdafx.h"

enum CopyOption { CopyValueFromRange, CopyValueFromSubset, DefaultCopy };

typedef uint32_t temporal_t;
typedef uint8_t categorical_t;

struct spatial_t {
  spatial_t() : spatial_t(0, 0, 0) {}

  spatial_t(uint32_t _x, uint32_t _y, uint8_t _z, uint8_t _l = 1)
      : type(1), x(_x), y(_y), z(_z), leaf(_l) {}

  inline bool operator==(const spatial_t& other) const {
    return (z == other.z && x == other.x && y == other.y);
  }

  inline bool contains(const spatial_t& other) const {
    if (other.z > z) {
      uint64_t n = (uint64_t)1 << (other.z - z);

      uint64_t x_min = x * n;
      uint64_t x_max = x_min + n;

      uint64_t y_min = y * n;
      uint64_t y_max = y_min + n;

      return x_min <= other.x && x_max >= other.x && y_min <= other.y &&
             y_max >= other.y;
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
      // TODO remove type hack
      uint64_t type : 2;
      uint64_t x : 25;
      uint64_t y : 25;
      uint64_t z : 5;
      uint64_t leaf : 1;
    };

    uint64_t data;
  };
};

struct region_t {
  region_t() = default;

  region_t(uint32_t _x0, uint32_t _y0, uint32_t _x1, uint32_t _y1, uint8_t _z)
      : x0(_x0), y0(_y0), x1(_x1), y1(_y1), z(_z) {}

  inline bool cover(const spatial_t& tile) const {
    if (z > tile.z) {
      uint64_t n = (uint64_t)1 << (z - tile.z);

      uint64_t x_min = tile.x * n;
      uint64_t x_max = x_min + n;

      uint64_t y_min = tile.y * n;
      uint64_t y_max = y_min + n;

      return (x_min >= x0 && y_min >= y0 && x_max <= x1 && y_max <= y1);
    } else
      return false;
  }

  inline bool intersect(const spatial_t& tile) const {
    if (z > tile.z) {
      uint64_t n = (uint64_t)1 << (z - tile.z);

      uint64_t x_min = tile.x * n;
      uint64_t x_max = x_min + n;

      uint64_t y_min = tile.y * n;
      uint64_t y_max = y_min + n;

      return (x0 <= x_max && y0 <= y_max && x1 >= x_min && y1 >= y_min);
    } else if (z == tile.z) {
      return x0 <= tile.x && y0 <= tile.y && x1 >= tile.x && y1 >= tile.y;
    } else {
      return false;
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const region_t& obj) {
    return os << obj.z << "/" << obj.x0 << "/" << obj.y0 << "/" << obj.x1 << "/"
              << obj.y1;
  }

  uint64_t x0, y0, x1, y1, z;
};

struct coordinates_t {
  float lat, lon;
};

struct BinaryHeader {
  uint32_t bytes;
  uint32_t records;
};

struct interval_t {
  interval_t() = default;

  interval_t(temporal_t lower, temporal_t upper) : bound{lower, upper} {};

  inline bool contain(const temporal_t& lower, const temporal_t& upper) const {
    return bound[0] <= lower && bound[1] >= upper;
  }

  friend std::ostream& operator<<(std::ostream& os, const interval_t& obj) {
    return os << obj.bound[0] << "/" << obj.bound[1];
  }

  std::array<temporal_t, 2> bound;
};
