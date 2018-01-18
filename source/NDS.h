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

  std::string query(const Query &query, std::ofstream *telemetry);

  interval_t get_interval() const;

  inline uint32_t size() const { return _root[0].back(); }

  inline Data *data() const { return _data_ptr.get(); }

  template<typename Container>
  inline static void swap_and_clear(Container &lhs, Container &rhs) {
    lhs.swap(rhs);
    rhs.clear();
  }

  inline pivot_ctn *create_link(bined_pivot_t &binned, const build_ctn &container) {
    pivot_ctn *link = new pivot_ctn(container.size());
    std::memcpy(&(*link)[0], &container[0], container.size() * sizeof(Pivot));

    // create payload for proper pivots
    for (auto &ptr : (*link)) {
      ptr.create_payload();
    }

    /*// create or copy payloads
    for (auto &ptr : (*link)) {

      payload_t *payload = nullptr;
      pivot_ctn::iterator it;

      for (auto &link_ctn : links) {
        it = std::find(link_ctn->begin(), link_ctn->end(), ptr);
        if (it != link_ctn->end()) {
          payload = (*it).get_payload();
          break;
        }
      }

      if (payload == nullptr) {
        ptr.create_payload();
      } else {
        ptr.copy_payload((*it));
      }
    }*/

    binned.proper = true;

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
      return create_link(binned, container);
    } else {
      return link;
    }
  }

  inline
  void share(bined_pivot_t &binned, const build_ctn &container,
             const link_ctn &links, link_ctn &share) {
    pivot_ctn *link = get_link(binned, container, links);

    binned.pivots = link;
    share.emplace_back(link);
  }

 private:
  pivot_ctn _root;
  std::unique_ptr<Data> _data_ptr;
  std::vector<std::pair<Dimension::Type, std::unique_ptr<Dimension>>>
      _dimension;
};
