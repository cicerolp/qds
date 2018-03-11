
#include "SpatialElement.h"

SpatialElement::SpatialElement(NDS &nds, Data &data, BuildPair<build_ctn> &range, BuildPair<link_ctn> &links,
                               const spatial_t &tile) {
  _el.value = tile.data;
  nds.share(data, _el, range.input, links);
}

SpatialElement::SpatialElement(NDS &nds, Data &data, const build_ctn &container, const link_ctn &links,
                               const spatial_t &tile) {
  _el.value = tile.data;
  _el.pivots = nds.get_link(data, _el, container, links);
}

uint32_t SpatialElement::expand(NDS &nds,
                                Data &data,
                                BuildPair<build_ctn> &range,
                                BuildPair<link_ctn> &links,
                                uint32_t bin) {
  spatial_t &value = (*reinterpret_cast<spatial_t *>(&_el.value));

  uint8_t next_level = value.z + 1;
  uint32_t pivots_count = static_cast<uint32_t>(_el.ptr().size());

  if (next_level < g_Quadtree_Depth && count_expand(bin)) {
    std::array<build_ctn, 4> tmp_ctn{};

    // node will be expanded
    value.leaf = 0;

    for (const auto &ptr : _el.ptr()) {
      std::array<uint32_t, 4> used{};

      for (auto i = ptr.front(); i < ptr.back(); ++i) {
        auto coords = data.record<coordinates_t>(i);

        auto y = mercator_util::lat2tiley(coords->lat, next_level);
        auto x = mercator_util::lon2tilex(coords->lon, next_level);
        auto index = mercator_util::index(x, y);

        data.setHash(i, index);
        ++used[index];
      }

      // sorting
      data.sort(ptr.front(), ptr.back());

      uint32_t accum = ptr.front();
      for (int i = 0; i < 4; ++i) {
        if (used[i] == 0) continue;

        uint32_t first = accum;
        accum += used[i];
        uint32_t second = accum;

        tmp_ctn[i].emplace_back(first, second);
      }
    }

    link_ctn parent{_el.pivots};

    for (uint32_t i = 0; i < 4; ++i) {
      if (tmp_ctn[i].size() == 0) {
        continue;
      }

      auto tile = get_tile(value.x * 2, value.y * 2, next_level, i);

      // share pivot between child and parent
      _container[i] = std::make_unique<SpatialElement>(nds, data, tmp_ctn[i], parent, tile);

      pivots_count += _container[i]->expand(nds, data, range, links, bin);
    }

  } else {
    links.output.emplace_back(_el.pivots);
    range.output.insert(range.output.end(), _el.ptr().begin(), _el.ptr().end());
  }

  return pivots_count;
}

void SpatialElement::query_tile(const spatial_t &tile, uint64_t resolution, bined_ctn &subset) const {
  const spatial_t &value = (*reinterpret_cast<const spatial_t *>(&_el.value));

  if (value.contains(tile)) {
    if (value.leaf || value.z == tile.z) {
      return aggregate_tile(tile.z + resolution, subset);
    } else {
      if (_container[0] != nullptr)
        _container[0]->query_tile(tile, resolution, subset);
      if (_container[1] != nullptr)
        _container[1]->query_tile(tile, resolution, subset);
      if (_container[2] != nullptr)
        _container[2]->query_tile(tile, resolution, subset);
      if (_container[3] != nullptr)
        _container[3]->query_tile(tile, resolution, subset);
    }
  }
}

void SpatialElement::query_region(const region_t &region, bined_ctn &subset) const {
  const spatial_t &value = (*reinterpret_cast<const spatial_t *>(&_el.value));

  if (region.intersect(value)) {
    if (value.z == region.z || value.leaf) {
      subset.emplace_back(&_el);
    } else if (region.cover(value)) {
      subset.emplace_back(&_el);
    } else {
      if (_container[0] != nullptr)
        _container[0]->query_region(region, subset);
      if (_container[1] != nullptr)
        _container[1]->query_region(region, subset);
      if (_container[2] != nullptr)
        _container[2]->query_region(region, subset);
      if (_container[3] != nullptr)
        _container[3]->query_region(region, subset);
    }
  }
}

void SpatialElement::aggregate_tile(uint64_t resolution, bined_ctn &subset) const {
  const spatial_t &value = (*reinterpret_cast<const spatial_t *>(&_el.value));

  if (value.z == resolution || value.leaf) {
    subset.emplace_back(&_el);
  } else {
    if (_container[0] != nullptr)
      _container[0]->aggregate_tile(resolution, subset);
    if (_container[1] != nullptr)
      _container[1]->aggregate_tile(resolution, subset);
    if (_container[2] != nullptr)
      _container[2]->aggregate_tile(resolution, subset);
    if (_container[3] != nullptr)
      _container[3]->aggregate_tile(resolution, subset);
  }
}
