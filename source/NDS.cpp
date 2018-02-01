#include "stdafx.h"
#include "types.h"

#include "NDS.h"

#include "Categorical.h"
#include "Spatial.h"
#include "Temporal.h"

NDS::NDS(const Schema &schema) {
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  start = std::chrono::high_resolution_clock::now();

  uint32_t pivots_count = 0;

  Data data(schema.file);

  std::cout << "NDS: " << std::endl;
  std::cout << "\tName: " << schema.name << std::endl;
  std::cout << "\tSize: " << data.size() << std::endl;
  std::cout << std::endl;

  _root = pivot_ctn(1);
  _root[0] = Pivot(0, data.size());

#ifdef NDS_ENABLE_PAYLOAD
  for (const auto &info : schema.payload) {
    switch (info.bin) {
      case 0: {
        std::cout << "\tPayload Dimension: \n\t\t" << info << std::endl;
        _payload.emplace_back(std::make_unique<PDigest>(info));
      }
        break;
    }
    // prepare payload
    data.preparePayload(info.offset);
  }
  std::cout << std::endl;

  // create root payload
  NDS::create_payload(data, _root[0]);
#endif // NDS_ENABLE_PAYLOAD

  BuildPair<link_ctn> links;
  links.input.emplace_back(&_root);

  BuildPair<build_ctn> range;
  range.input.emplace_back(0, data.size());

  for (const auto &info : schema.dimension) {
    switch (info.type) {
      case DimensionSchema::Spatial: {
        std::cout << "\tSpatial Dimension: \n\t\t" << info << std::endl;
        _dimension.emplace_back(std::make_unique<Spatial>(info));
      }
        break;
      case DimensionSchema::Temporal: {
        std::cout << "\tTemporal Dimension: \n\t\t" << info << std::endl;
        _dimension.emplace_back(std::make_unique<Temporal>(info));
      }
        break;
      case DimensionSchema::Categorical: {
        std::cout << "\tCategorical Dimension: \n\t\t" << info << std::endl;
        _dimension.emplace_back(std::make_unique<Categorical>(info));
      }
        break;
      default:std::cerr << "error: invalid NDS" << std::endl;
        std::abort();
        break;
    }

    uint32_t curr_count = _dimension.back()->build(*this, data, range, links);
    pivots_count += curr_count;

    swap_and_clear<link_ctn>(links.input, links.output);
    swap_and_clear<build_ctn>(range.input, range.output);

    std::cout << "\t\tNumber of Pivots: " + std::to_string(curr_count) << std::endl;
  }

  std::cout << "\n\tTotal Number of Pivots: " << pivots_count << std::endl;

  end = std::chrono::high_resolution_clock::now();
  long long duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

  std::cout << "\tDuration: " + std::to_string(duration) + "s\n" << std::endl;
}

std::string NDS::query(const Query &query) {
  subset_ctn subsets;

  RangePivot root(_root[0]);

  for (auto &d : _dimension) {
    if (d->query(query, subsets) == false) {
      // empty query
      root.pivot.back(0);
      subsets.clear();
      break;
    }
  }

  return Dimension::serialize(query, subsets, root);
}
