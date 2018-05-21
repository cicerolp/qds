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

inline uint32_t lon2tilex(double lon, int z) {
#ifdef NDS_ENABLE_CRS_SIMPLE
  uint32_t x = (lon + 512.0) / 1024.0 * (1 << z);
  return x & ((1 << z) - 1);
#else
  uint32_t x = (lon + 180.0) / 360.0 * (1 << z);
  return x & ((1 << z) - 1);
#endif // NDS_ENABLE_CRS_SIMPLE
}

inline uint32_t lat2tiley(double lat, int z) {
#ifdef NDS_ENABLE_CRS_SIMPLE
  uint32_t y = (lat + 512.0) / 1024.0 * (1 << z);
  return y & ((1 << z) - 1);
#else
  static const double PI_180 = M_PI / 180.0;
  uint32_t y = (1.0 - log(tan(lat * PI_180) + 1.0 / cos(lat * PI_180)) / M_PI) / 2.0 * (1 << z);
  return y & ((1 << z) - 1);
#endif // NDS_ENABLE_CRS_SIMPLE
}

inline float tilex2lon(double x, int z) {
  return static_cast<float>(x / (1 << z) * 360.0 - 180);
}

inline float tiley2lat(double y, int z) {
  double n = M_PI - 2.0 * M_PI * y / (1 << z);
  return static_cast<float>(180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n))));
}

}  // namespace mercator_util
