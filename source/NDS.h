#pragma once

#include "Data.h"
#include "Payload.h"
#include "Dimension.h"
#include "Pivot.h"
#include "Query.h"
#include "Pipeline.h"
#include "Schema.h"

using AggrNoneCtn = std::vector<std::shared_ptr<AggrNone>>;
using AggrRangeCtn = std::vector<std::shared_ptr<AggrRange>>;
using AggrSubsetCtn = std::vector<std::shared_ptr<AggrSubset>>;

class NDS {
 public:
  NDS(const Schema &schema);

  NDS(const NDS &that) = delete;
  NDS &operator=(NDS const &) = delete;

  std::string query(const Query &query);
  std::string pipeline(const Pipeline &pipeline);

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
    size_t accumulated_size = 0;
    std::vector<std::vector<float>> payload_buffer;

    // accumulate payloads
    for (auto i = 0; i < _payload.size(); ++i) {
      payload_buffer.emplace_back(_payload[i]->get_payload(data, pivot));
      accumulated_size += payload_buffer.back().size();
    }

    // allocate payload
    payload_ctn payloads = new float[accumulated_size + _payload.size() + 1];

    // insert indexes
    size_t accum_index = _payload.size() + 1;
    for (auto i = 0; i < _payload.size(); ++i) {
      accum_index += payload_buffer[i].size();

      payloads[i + 1] = accum_index;
    }
    payloads[0] = _payload.size() + 1;

    // insert data
    size_t index = _payload.size() + 1;
    for (auto i = 0; i < _payload.size(); ++i) {
      // copy data
      std::memcpy(&(payloads[index]), &(payload_buffer[i])[0], payload_buffer[i].size() * sizeof(float));

      index += payload_buffer[i].size();
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
  std::string serialize_pipeline(const Pipeline &pipeline,
                                 subset_ctn &source_ctn,
                                 subset_ctn &dest_ctn,
                                 const RangePivot &root) const;

  range_ctn get_range(const subset_ctn &subsets) const;

  bined_ctn get_subset(const subset_ctn &subsets) const;

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

  // group_by

  template<typename T>
  struct GroupBy {
    GroupBy() = default;
    GroupBy(const T &__aggrs, const range_ctn &__range, const bined_ctn &__subset)
        : aggrs(__aggrs), range(__range), subset(__subset) {}

    T aggrs;
    range_ctn range;
    bined_ctn subset;
  };

  template<typename T>
  using GroupCtn = std::pair<GroupBy<T>, GroupBy<T>>;

  void group_by_subset(AggrSubsetCtn &aggrs, json &writer, range_ctn &range, const bined_ctn &subset) const;

  void group_by_range(AggrRangeCtn &aggrs, json &writer, range_ctn &range, const bined_ctn &subset) const;

  void summarize_subset(AggrNoneCtn &aggrs, json &writer, range_ctn &range, const bined_ctn &subset) const;

  void summarize_range(AggrNoneCtn &aggrs, json &writer, range_ctn &range) const;

  void summarize_pipe(GroupCtn<AggrNoneCtn> &groups, json &writer) const;

  void do_summarize_range(AggrNoneCtn &aggrs, range_ctn &range) const;
  void do_summarize_subset(AggrNoneCtn &aggrs, range_ctn &range, const bined_ctn &subset) const;


  // get_aggr

  AggrNoneCtn get_aggr_none(const Query &query) const;

  AggrRangeCtn get_aggr_range(const Query &query) const;

  AggrSubsetCtn get_aggr_subset(const Query &query, size_t subset_size) const;

  pivot_ctn _root;
  std::vector<std::unique_ptr<Payload>> _payload;
  std::unordered_map<std::string, size_t> _payload_index;

  std::vector<std::unique_ptr<Dimension>> _dimension;
};
