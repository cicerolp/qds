#pragma once

#include "Data.h"
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

  interval_t get_interval() const;

  inline uint32_t size() const { return _root[0].back(); }

  inline Data *data() const { return _data_ptr.get(); }

  template<typename Container>
  inline static void swap_and_clear(Container &lhs, Container &rhs) {
    lhs.swap(rhs);
    rhs.clear();
  }

#ifdef ENABLE_PDIGEST
  template<class InputIt1, class InputIt2>
  inline void share_payload(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) {
    while (first1 != last1 && first2 != last2) {
      if ((*first1).front() < (*first2).front()) {
        ++first1;
      } else {
        if ((*first2).back() < (*first1).back()) {
          ++first2;
        } else {
          first2->set_payload(*first1);
          ++first1;
          ++first2;
        }
      }
    }
  }
#endif

  inline pivot_ctn *create_link(bined_pivot_t &binned, const build_ctn &container, const link_ctn &links) {
    pivot_ctn *link = new pivot_ctn(container.size());
    std::memcpy(&(*link)[0], &container[0], container.size() * sizeof(Pivot));

#ifdef ENABLE_PDIGEST
    for (auto &link_ctn : links) {
      share_payload(link_ctn->begin(), link_ctn->end(), link->begin(), link->end());
    }

    for (auto &ptr : *link) {
      if (ptr.get_payload() == nullptr) {
        ptr.create_payload(*this);
      }
    }
#endif
    return link;
  }

  inline pivot_ctn *get_link(bined_pivot_t &binned, const build_ctn &container, const link_ctn &links) {
    pivot_ctn *link = nullptr;

    for (auto &ptr : links) {
      if (ptr->size() == container.size() && std::equal(container.begin(), container.end(), (*ptr).begin())) {
        link = ptr;
        break;
      }
    }

    if (link == nullptr) {
      return create_link(binned, container, links);
    } else {
      return link;
    }
  }

  inline
  void share(bined_pivot_t &binned, const build_ctn &container, const link_ctn &links, link_ctn &share) {
    pivot_ctn *link = get_link(binned, container, links);

    binned.pivots = link;
    share.emplace_back(link);
  }

 private:
  pivot_ctn _root;
  std::unique_ptr<Data> _data_ptr;
  std::vector<std::pair<Dimension::Type, std::unique_ptr<Dimension>>> _dimension;
};
