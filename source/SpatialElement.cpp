﻿
#include "SpatialElement.h"

SpatialElement::SpatialElement(const spatial_t& tile, pivot_ctn* ptr) {
  el.value = tile.data;
  el.pivots = ptr;
}

SpatialElement::SpatialElement(const spatial_t& tile, const build_ctn& range,
                               NDS& nds) {
  el.value = tile.data;
  el.pivots = nds.create_link(el, range);
}

SpatialElement::SpatialElement(const spatial_t& tile, const build_ctn& range,
                               const link_ctn& links, NDS& nds) {
  el.value = tile.data;
  el.pivots = nds.get_link(el, range, links);
}

uint32_t SpatialElement::expand(build_ctn& response, uint32_t bin,
                                link_ctn& share, NDS& nds) {
  spatial_t& value = (*reinterpret_cast<spatial_t*>(&el.value));

  uint8_t next_level = value.z + 1;
  uint32_t pivots_count = static_cast<uint32_t>(el.ptr().size());

  if (next_level < g_Quadtree_Depth && count_expand(bin)) {
    std::array<build_ctn, 4> tmp_ctn{};

    // node will be expanded
    value.leaf = 0;

    for (const auto& ptr : el.ptr()) {
      std::array<uint32_t, 4> used{};

      for (auto i = ptr.front(); i < ptr.back(); ++i) {
        auto coords = nds.data()->record<coordinates_t>(i);

        auto y = mercator_util::lat2tiley(coords->lat, next_level);
        auto x = mercator_util::lon2tilex(coords->lon, next_level);
        auto index = mercator_util::index(x, y);

        nds.data()->setHash(i, index);
        ++used[index];
      }

      // sorting
      nds.data()->sort(ptr.front(), ptr.back());

      uint32_t accum = ptr.front();
      for (int i = 0; i < 4; ++i) {
        if (used[i] == 0) continue;

        uint32_t first = accum;
        accum += used[i];
        uint32_t second = accum;

        tmp_ctn[i].emplace_back(first, second);
      }
    }

    uint32_t index = 0, nodes_count = 0;
    for (uint32_t i = 0; i < 4; ++i) {
      if (tmp_ctn[i].size() != 0) {
        ++nodes_count;
        index = i;
      }
    }

    if (nodes_count == 1) {
      auto tile = get_tile(value.x * 2, value.y * 2, index);
      _container[index] = std::make_unique<SpatialElement>(
          spatial_t(tile.first, tile.second, next_level), el.pivots);
      pivots_count += _container[index]->expand(response, bin, share, nds);

    } else {
      for (uint32_t i = 0; i < 4; ++i) {
        if (tmp_ctn[i].size() == 0) continue;
        auto tile = get_tile(value.x * 2, value.y * 2, i);
        _container[i] = std::make_unique<SpatialElement>(
            spatial_t(tile.first, tile.second, next_level), tmp_ctn[i], nds);
        pivots_count += _container[i]->expand(response, bin, share, nds);
      }
    }

  } else {
    share.emplace_back(el.pivots);
    response.insert(response.end(), el.ptr().begin(), el.ptr().end());
  }

  return pivots_count;
}

void SpatialElement::query_tile(const spatial_t& tile, uint64_t resolution,
                                subset_pivot_ctn& subset, uint64_t zoom) const {
  const spatial_t& value = (*reinterpret_cast<const spatial_t*>(&el.value));

  if (value.contains(tile)) {
    if (value.leaf || zoom == tile.z) {
      return aggregate_tile(tile.z + resolution, subset, zoom);
    } else {
      if (_container[0] != nullptr)
        _container[0]->query_tile(tile, resolution, subset, zoom + 1);
      if (_container[1] != nullptr)
        _container[1]->query_tile(tile, resolution, subset, zoom + 1);
      if (_container[2] != nullptr)
        _container[2]->query_tile(tile, resolution, subset, zoom + 1);
      if (_container[3] != nullptr)
        _container[3]->query_tile(tile, resolution, subset, zoom + 1);
    }
  }
}

void SpatialElement::query_region(const region_t& region, subset_pivot_ctn& subset,
                                  uint64_t zoom) const {
  const spatial_t& value = (*reinterpret_cast<const spatial_t*>(&el.value));

  if (region.intersect(value)) {
    if (zoom == region.z || value.leaf) {
      subset.emplace_back(&el);
    } else if (region.cover(value)) {
      subset.emplace_back(&el);
    } else {
      if (_container[0] != nullptr)
        _container[0]->query_region(region, subset, zoom + 1);
      if (_container[1] != nullptr)
        _container[1]->query_region(region, subset, zoom + 1);
      if (_container[2] != nullptr)
        _container[2]->query_region(region, subset, zoom + 1);
      if (_container[3] != nullptr)
        _container[3]->query_region(region, subset, zoom + 1);
    }
  }
}

void SpatialElement::aggregate_tile(uint64_t resolution, subset_pivot_ctn& subset,
                                    uint64_t zoom) const {
  const spatial_t& value = (*reinterpret_cast<const spatial_t*>(&el.value));

  if (zoom == resolution || value.leaf) {
    subset.emplace_back(&el);
  } else {
    if (_container[0] != nullptr)
      _container[0]->aggregate_tile(resolution, subset, zoom + 1);
    if (_container[1] != nullptr)
      _container[1]->aggregate_tile(resolution, subset, zoom + 1);
    if (_container[2] != nullptr)
      _container[2]->aggregate_tile(resolution, subset, zoom + 1);
    if (_container[3] != nullptr)
      _container[3]->aggregate_tile(resolution, subset, zoom + 1);
  }
}
