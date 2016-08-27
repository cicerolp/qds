#pragma once

namespace mercator_util {
   inline uint32_t index(uint32_t x, uint32_t y) {
      if (x % 2 == 0) {
         if (y % 2 == 0) return 0;
         else return 1;
      } else {
         if (y % 2 == 0) return 2;
         else return 3;
      }
   }

   inline uint32_t lon2tilex(double lon, int z) {
      uint32_t n = 1 << z;
      return static_cast<uint32_t>((lon + 180.0) / 360.0 * n) % n;
   }

   inline uint32_t lat2tiley(double lat, int z) {
      static const double PI_4 = M_PI / 4.0;
      uint32_t n = 1 << z;      
      uint32_t y = (1.0 - (log(tan(PI_4 + (lat / 180.0 * M_PI) / 2.0)) / M_PI)) / 2.0 * n;
      return y % n;
   }

   inline float tilex2lon(double x, int z) {
      return static_cast<float>(x / (1 << z) * 360.0 - 180);
   }

   inline float tiley2lat(double y, int z) {
      double n = M_PI - 2.0 * M_PI * y / (1 << z);
      return static_cast<float>(180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n))));
   }

} // namespace mercator_util
