#pragma once
#include "stdafx.h"

namespace mercator_util {
   inline uint8_t get_index(uint32_t x, uint32_t y) {
      if (x % 2 == 0) {
         if (y % 2 == 0) return 0;
         else return 1;
      } else {
         if (y % 2 == 0) return 2;
         else return 3;
      }
   }

   inline uint32_t lon2tilex(double lon, int z) {
      //lon = std::max(std::min(lon, 179.8), -179.8);
      int x = static_cast<int>(std::floor((lon + 180.0) / 360.0 * pow(2.0, z)));
      return x & ((1 << z) - 1);
   }

   inline uint32_t lat2tiley(double lat, int z) {
      //lat = std::max(std::min(lat, 89.8), -89.8);
      int y = static_cast<int>(std::floor((1.0 - log(tan(lat * M_PI / 180.0) + 1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * pow(2.0, z)));
      return y & ((1 << z) - 1);
   }

   inline float tilex2lon(double x, int z) {
      return static_cast<float>(x / pow(2.0, z) * 360.0 - 180);
   }

   inline float tiley2lat(double y, int z) {
      double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
      return static_cast<float>(180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n))));
   }

   /*inline bool intersectsTile(const spatial_t& tile, const TileBounds& bounds) {
      return (bounds.lat1 <= tiley2lat(tile.y, tile.z)) && (bounds.lon1 >= tilex2lon(tile.x, tile.z))
         && (bounds.lat0 >= tiley2lat(tile.y + 1, tile.z)) && (bounds.lon0 <= tilex2lon(tile.x + 1, tile.z));
   }*/

   /*inline bool intersects(const spatial_t& tile0, const Tile& tile1) {
      const float lat = tiley2lat(tile0.y, tile0.z);
      const float lon = tilex2lon(tile0.x, tile0.z);
      return (lon >= tile1.lon0 && lon <= tile1.lon1) && (lat <= tile1.lat0 && lat >= tile1.lat1);
   }*/
} // namespace mercator_util
