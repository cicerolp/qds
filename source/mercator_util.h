#pragma once

namespace mercator_util {
inline uint32_t index(uint32_t x, uint32_t y) {
  if (x % 2 == 0) {
    if (y % 2 == 0)
      return 0;
    else
      return 1;
  } else {
    if (y % 2 == 0)
      return 2;
    else
      return 3;
  }
}

inline uint32_t get_tile_x(double min, double max, double value, int z) {
  uint32_t x = (value + std::abs(min)) / (std::abs(min) + max) * (1 << z);
  return x & ((1 << z) - 1);
}

inline uint32_t get_tile_y(double min, double max, double value, int z) {
  uint32_t x = (value + std::abs(min)) / (std::abs(min) + max) * (1 << z);
  return x & ((1 << z) - 1);
}

inline uint32_t lon_to_tile_x(double lon, int z) {
  uint32_t x = (lon + 180.0) / 360.0 * (1 << z);
  return x & ((1 << z) - 1);
}

inline uint32_t lat_to_tile_y(double lat, int z) {
  static const double PI_180 = M_PI / 180.0;
  uint32_t y = (1.0 - log(tan(lat * PI_180) + 1.0 / cos(lat * PI_180)) / M_PI) / 2.0 * (1 << z);
  return y & ((1 << z) - 1);
}

}  // namespace mercator_util
