#pragma once

#include "Data.h"
#include "Payload.h"
#include "Dimension.h"
#include "Pivot.h"
#include "Query.h"
#include "Schema.h"

class NDS {
 public:
  NDS(const Schema &schema);

  NDS(const NDS &that) = delete;
  NDS &operator=(NDS const &) = delete;

  std::string query(const Query &query);
  std::string schema() const;

  template<typename Container>
  inline static void swap_and_clear(Container &lhs, Container &rhs) {
    lhs.swap(rhs);
    rhs.clear();
  }

#ifdef NDS_SHARE_PAYLOAD
  template<class InputIt1, class InputIt2>
  inline void share_payload(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) {
    while (first1 != last1 && first2 != last2) {
      if ((*first1).front() < (*first2).front()) {
        ++first1;
      } else {
        if ((*first2).back() < (*first1).back()) {
          ++first2;
        } else {
          first2->set_payload_ptr(first1->get_payload_ptr());
          ++first1;
          ++first2;
        }
      }
    }
  }
#endif // NDS_SHARE_PAYLOAD

#ifdef NDS_ENABLE_PAYLOAD
  inline void create_payload(Data &data, Pivot &pivot) {
    payload_ctn payloads = new payload_t *[_payload.size()];

    for (auto i = 0; i < _payload.size(); ++i) {
      auto raw_data = _payload[i]->get_payload(data, pivot);

      payloads[i] = new payload_t(raw_data.size());

      // copy data
      std::memcpy(&(*payloads[i])[0], &raw_data[0], raw_data.size() * sizeof(float));
    }

    pivot.set_payload_ptr(payloads);
  }
#endif // NDS_ENABLE_PAYLOAD

  inline pivot_ctn *create_link(Data &data, bined_pivot_t &binned, const build_ctn &ctn, const link_ctn &links) {
    pivot_ctn *link = new pivot_ctn(ctn.size());
    std::memcpy(&(*link)[0], &ctn[0], ctn.size() * sizeof(Pivot));

#ifdef NDS_ENABLE_PAYLOAD
#ifdef NDS_SHARE_PAYLOAD
    for (auto &link_ctn : links) {
      share_payload(link_ctn->begin(), link_ctn->end(), link->begin(), link->end());
    }
#endif // NDS_SHARE_PAYLOAD

    for (auto &ptr : *link) {
      if (ptr.get_payload_ptr() == nullptr) {
        create_payload(data, ptr);
      }
    }
#endif // NDS_ENABLE_PAYLOAD
    return link;
  }

  inline pivot_ctn *get_link(Data &data, bined_pivot_t &binned, const build_ctn &ctn, const link_ctn &links) {
#ifdef NDS_SHARE_PIVOT
    pivot_ctn *link = nullptr;

    for (auto &ptr : links) {
      if (ptr->size() == ctn.size() && std::equal(ctn.begin(), ctn.end(), (*ptr).begin())) {
        link = ptr;
        break;
      }
    }

    if (link == nullptr) {
      return create_link(data, binned, ctn, links);
    } else {
      return link;
    }
#else
    return create_link(data, binned, container, links);
#endif // NDS_SHARE_PIVOT
  }

  inline void share(Data &data, bined_pivot_t &binned, const build_ctn &ctn, BuildPair<link_ctn> &links) {
    auto *link = get_link(data, binned, ctn, links.input);

    binned.pivots = link;
    links.output.emplace_back(link);
  }

 private:
  std::string serialize(const Query &query, subset_ctn &subsets, const RangePivot &root) const;

  void restrict(range_ctn &range, range_ctn &response, const subset_t &subset, CopyOption &option) const;

  bool search_iterators(range_it &it_range, const range_ctn &range,
                        pivot_it &it_lower, pivot_it &it_upper, const pivot_ctn &subset) const;

  void compactation(range_ctn &input, range_ctn &output, CopyOption option) const;

  inline void swap_and_sort(range_ctn &range, range_ctn &response, CopyOption option) const {
    // according to benchmark, this is a bit slower than std::sort() on randomized
    // sequences,  but much faster on partially - sorted sequences
    gfx::timsort(response.begin(), response.end());

    range.clear();

    // compaction (and swap) phase
    compactation(response, range, option);

    response.clear();
  }

  inline size_t get_payload_index(const Query::clausule &clausule) const {
    size_t index = 0;

    if (_payload_index.size() != 0) {
      try {
        if (!clausule.first.empty()) {
          index = _payload_index.at(clausule.first);
        }
      } catch (std::out_of_range) {
        std::cerr << "error: invalid aggr" << std::endl;
      }
    }
    return index;
  }

  void group_by_subset(aggrs_subset_t &aggrs, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                       range_ctn &range, const subset_pivot_ctn &subset) const;

  void group_by_range(aggrs_range_t &aggrs, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                      range_ctn &range, const subset_pivot_ctn &subset) const;

  void group_by_none(aggrs_none_t &aggrs, rapidjson::Writer<rapidjson::StringBuffer> &writer,
                     range_ctn &range, const subset_pivot_ctn &subset) const;

  void group_by_none(aggrs_none_t &aggrs, rapidjson::Writer<rapidjson::StringBuffer> &writer, range_ctn &range) const;

  pivot_ctn _root;
  std::vector<std::unique_ptr<Payload>> _payload;
  std::unordered_map<std::string, size_t> _payload_index;

  std::vector<std::unique_ptr<Dimension>> _dimension;
};
