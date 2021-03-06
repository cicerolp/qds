#pragma once

#include "Data.h"
#include "Payload.h"
#include "Dimension.h"
#include "Pivot.h"

#include "Schema.h"

#include "Query.h"
#include "Pipeline.h"
#include "Clustering.h"
#include "AugmentedSeries.h"

#include "Schema.h"
#include "Server.h"

using AggrSummarizeCtn = std::vector<std::shared_ptr<AggrSummarize>>;
using AggrGroupByCtn = std::vector<std::shared_ptr<AggrGroupBy>>;

class NDS {
 public:
  NDS(const Schema &schema, const Server::server_opts &opts);

  NDS(const NDS &that) = delete;
  NDS &operator=(NDS const &) = delete;

  std::string query(const Query &query);
  std::string pipeline(const Pipeline &pipeline);
  std::string clustering(const Clustering &clustering);
  std::string augmented_series(const AugmentedSeries &augmented_series);

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
      // shared pivots
      SHARED_PIVOTS_INCR(link->size())

      return link;
    }
#else
    return create_link(data, binned, ctn, links);
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
                                 const subset_ctn &source_ctn,
                                 const subset_ctn &dest_ctn,
                                 const RangePivot &root) const;

  range_ctn get_range(const subset_ctn &subsets, CopyOption &option) const;

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
        std::cerr << "[error] invalid aggr: " << clausule.first << std::endl;
      }
    }
    return index;
  }

  inline bool validation(const Query &query, subset_ctn &ctn) const {
    // validadion phase
    for (auto &d : _dimension) {
      if (!d->query(query, ctn)) {
        ctn.clear();
        return false;
      }
    }

    return true;
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

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // common methods
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void do_summarize(AggrSummarizeCtn &aggrs, const range_ctn &range, const bined_ctn &subset) const;
  void do_group_by(AggrGroupByCtn &aggrs, const range_ctn &range,
                   const bined_ctn &subset, const CopyOption &option) const;

  AggrSummarizeCtn get_aggr_summarize_ctn(const Query &query) const;
  AggrGroupByCtn get_aggr_group_by_ctn(const Query &query) const;

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // regular queries
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void summarize_query(const AggrSummarizeCtn &aggrs, json &writer) const;

  void group_by_query(const AggrGroupByCtn &aggrs, json &writer) const;

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // group_by data queries
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void group_by_data(const AggrGroupByCtn &aggrs, json &writer) const;
  void group_by_data(const AggrGroupByCtn &aggrs, std::vector<uint64_t> &raw) const;

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // pipeline queries
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void summarize_pipe(const GroupCtn<AggrSummarizeCtn> &groups, json &writer) const;
  void summarize_pipe(const GroupCtn<AggrSummarizeCtn> &groups, std::vector<float> &raw) const;

  void group_by_inner_join(const GroupCtn<AggrGroupByCtn> &groups, json &writer, uint32_t threshold) const;
  void group_by_inner_join(const GroupCtn<AggrGroupByCtn> &groups, std::vector<float> &raw, uint32_t threshold) const;

  void group_by_left_join(const GroupCtn<AggrGroupByCtn> &groups, json &writer, uint32_t threshold) const;
  void group_by_left_join(const GroupCtn<AggrGroupByCtn> &groups, std::vector<float> &raw, uint32_t threshold) const;

  void group_by_right_join(const GroupCtn<AggrGroupByCtn> &groups, json &writer, uint32_t threshold) const;
  void group_by_right_join(const GroupCtn<AggrGroupByCtn> &groups, std::vector<float> &raw, uint32_t threshold) const;

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // equality queries
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void summarize_equal(const GroupCtn<AggrSummarizeCtn> &groups, std::vector<float> &raw) const;

  void left_join_equal(const GroupCtn<AggrGroupByCtn> &groups, std::vector<float> &raw) const;
  void right_join_equal(const GroupCtn<AggrGroupByCtn> &groups, std::vector<float> &raw) const;

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // clustering
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  std::vector<std::vector<uint16_t>> initialize_clusters(const Clustering &clustering, size_t n_objs);

  GroupBy<AggrSummarizeCtn> get_cluster_summarize(const Query &query) const;
  GroupBy<AggrGroupByCtn> get_cluster_grop_by(const Query &query) const;

  std::vector<float> get_raw_by_summarize(const Query &query, const std::vector<GroupBy<AggrSummarizeCtn>> &clusters);
  std::vector<float> get_raw_by_group(const Query &query, const std::vector<GroupBy<AggrGroupByCtn>> &clusters);
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  pivot_ctn _root;
  std::vector<std::unique_ptr<Payload>> _payload;
  std::unordered_map<std::string, size_t> _payload_index;

  std::vector<std::unique_ptr<Dimension>> _dimension;
  std::unordered_map<std::string, size_t> _dimension_index;
};
