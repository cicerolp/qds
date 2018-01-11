#include "Temporal.h"
#include "NDS.h"

Temporal::Temporal(const std::tuple<uint32_t, uint32_t, uint32_t> &tuple)
    : Dimension(tuple) {}

uint32_t Temporal::build(const build_ctn &range, build_ctn &response,
                         const link_ctn &links, link_ctn &share, NDS &nds) {
  nds.data()->prepareOffset<temporal_t>(_offset);

  uint32_t pivots_count = 0;

  std::map<temporal_t, std::vector<Pivot>> tmp_ctn;

  for (const auto &ptr : range) {
    std::map<temporal_t, uint32_t> used;

    for (auto i = ptr.front(); i < ptr.back(); ++i) {
      auto value = (*nds.data()->record<temporal_t>(i));
      value = static_cast<temporal_t>(value / static_cast<float>(_bin)) * _bin;

      nds.data()->setHash(i, value);
      ++used[value];
    }

    uint32_t accum = ptr.front();
    for (const auto &entry : used) {
      uint32_t first = accum;
      accum += entry.second;
      uint32_t second = accum;

      response.emplace_back(first, second);
      tmp_ctn[entry.first].emplace_back(response.back());

      pivots_count++;
    }

    nds.data()->sort(ptr.front(), ptr.back());
  }

  _container = stde::dynarray<TemporalElement>(tmp_ctn.size());

  uint32_t index = 0;
  for (auto &pair : tmp_ctn) {
    _container[index].el.value = pair.first;
    nds.share(_container[index].el, pair.second, links, share);
    ++index;
  }

  nds.data()->dispose();

  return pivots_count;
}

bool Temporal::query(const Query &query, subset_ctn &subsets) const {
  auto clausule = query.get_const(std::to_string(_key));

  if (clausule != nullptr) {
    if (clausule->first == "interval") {
      auto interval = parse_interval(clausule->second);

      subset_t subset;

      if (query.get_group(std::to_string(_key))) {
        subset.option = CopyValueFromSubset;

        if (interval.contain(_container.front().el.value, _container.back().el.value)) {
          // all values selected
          for (const auto &it : _container) {
            subset.container.emplace_back(&it.el);
          }
        } else {
          auto it_lower_data = std::lower_bound(_container.begin(), _container.end(), interval.bound[0]);
          auto it_upper_date = std::lower_bound(it_lower_data, _container.end(), interval.bound[1]);

          for (auto it = it_lower_data; it < it_upper_date; ++it) {
            subset.container.emplace_back(&(*it).el);
          }
        }

      } else {
        if (interval.contain(_container.front().el.value, _container.back().el.value)) {
          // all values selected
          return true;
        } else {
          auto it_lower_data = std::lower_bound(_container.begin(), _container.end(), interval.bound[0]);
          auto it_upper_date = std::lower_bound(it_lower_data, _container.end(), interval.bound[1]);

          for (auto it = it_lower_data; it < it_upper_date; ++it) {
            subset.container.emplace_back(&(*it).el);
          }
        }
      }

      subsets.emplace_back(subset);
      return true;

    } else if (clausule->first == "sequence") {

      auto sequence = parse_sequence(clausule->second);

      subset_t subset;

      if (query.get_group(std::to_string(_key))) {
        subset.option = CopyValueFromSubset;
      }

      auto it_lower = _container.begin();
      auto it_upper = _container.begin();

      for (auto& interval : sequence.bounds) {

        it_lower = std::lower_bound(it_upper, _container.end(), interval.bound[0]);
        it_upper  = std::lower_bound(it_lower, _container.end(), interval.bound[1]);

        for (auto it = it_lower; it < it_upper; ++it) {
          subset.container.emplace_back(&(*it).el);
        }
      }

      subsets.emplace_back(subset);
      return true;
    }
  } else {
    return true;
  }
}
interval_t Temporal::parse_interval(const std::string &str) const {
  auto clausule = boost::trim_copy_if(str, boost::is_any_of("()"));

  if (clausule == "all") {
    return interval_t(_container.front().el.value, _container.back().el.value);
  } else {
    std::vector<std::string> tokens;
    boost::split(tokens, clausule, boost::is_any_of(":"));

    return interval_t(std::stoi(tokens[0]), std::stoi(tokens[1]));
  }
}
sequence_t Temporal::parse_sequence(const std::string &str) const {
  auto clausule = boost::trim_copy_if(str, boost::is_any_of("()"));

  std::vector<std::string> tokens;
  boost::split(tokens, clausule, boost::is_any_of(":"));

  return sequence_t(std::stoi(tokens[0]), std::stoi(tokens[1]), std::stoi(tokens[2]), std::stoi(tokens[3]));
}
