#include "Categorical.h"
#include "NDS.h"

Categorical::Categorical(const DimensionSchema &schema)
    : Dimension(schema), _container(schema.bin) {}

uint32_t Categorical::build(NDS &nds, Data &data, BuildPair<build_ctn> &range, BuildPair<link_ctn> &links) {
  data.prepareOffset<categorical_t>(_schema.offset);

  uint32_t pivots_count = 0;

  std::vector<build_ctn> tmp_ctn(_schema.bin);

  for (const auto &ptr : range.input) {
    std::vector<uint32_t> used(_schema.bin, 0);

    for (auto i = ptr.front(); i < ptr.back(); ++i) {
      auto value = (*data.record<categorical_t>(i));

      data.setHash(i, value);
      ++used[value];
    }

    // sort before tdigesting...
    data.sort(ptr.front(), ptr.back());

    uint32_t accum = ptr.front();
    for (uint32_t i = 0; i < _schema.bin; ++i) {
      if (used[i] == 0) continue;

      uint32_t first = accum;
      accum += used[i];
      uint32_t second = accum;

      range.output.emplace_back(first, second);
      tmp_ctn[i].emplace_back(range.output.back());

      ++pivots_count;
    }
  }

  for (uint32_t index = 0; index < _schema.bin; ++index) {
    _container[index].value = index;
    nds.share(data, _container[index], tmp_ctn[index], links);
  }

  return pivots_count;
}

bool Categorical::query(const Query &query, subset_ctn &subsets) const {
  auto clausule = query.get_const(_schema.index);

  if (clausule != nullptr) {
    auto values = parse(clausule->second);

    subset_t subset;

    if (query.group_by(_schema.index)) {
      subset.option = CopyValueFromSubset;

      if (values.size() == _schema.bin) {
        // all values selected
        for (const auto &el : _container) {
          if (!el.pivots->empty()) {
            subset.container.emplace_back(&el);
          }
        }
      } else {
        for (auto &value : values) {
          if (value < _container.size() && !_container[value].pivots->empty()) {
            subset.container.emplace_back(&_container[value]);
          }
        }
      }

    } else {
      if (values.size() == _schema.bin) {
        // all values selected
        return true;
      } else {
        for (auto &value : values) {
          if (value < _container.size() && !_container[value].pivots->empty()) {
            subset.container.emplace_back(&_container[value]);
          }
        }
      }
    }

    if (!subset.container.empty()) {
      subsets.emplace_back(subset);
      return true;
    } else {
      return false;
    }
  }

  return true;
}
std::vector<categorical_t> Categorical::parse(const std::string &str) const {
  auto clausule = boost::trim_copy_if(str, boost::is_any_of("()"));

  if (clausule == "all") {
    std::vector<categorical_t> values(_schema.bin);
    std::iota(values.begin(), values.end(), 0);
    return values;

  } else {
    boost::char_separator<char> sep(":");
    boost::tokenizer<boost::char_separator<char> > tokens(clausule, sep);

    std::vector<categorical_t> values;
    for (auto &v : tokens) {
      values.emplace_back(std::stoi(v));
    }
    return values;
  }
}
