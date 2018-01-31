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

  template<typename Container>
  inline static void swap_and_clear(Container &lhs, Container &rhs) {
    lhs.swap(rhs);
    rhs.clear();
  }

#ifdef NDS_SHARE_PAYLOAD
  template<class InputIt1, class InputIt2>
  inline static void share_payload(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) {
    while (first1 != last1 && first2 != last2) {
      if ((*first1).front() < (*first2).front()) {
        ++first1;
      } else {
        if ((*first2).back() < (*first1).back()) {
          ++first2;
        } else {
          first2->set_payload(first1->get_payload());
          ++first1;
          ++first2;
        }
      }
    }
  }
#endif // NDS_SHARE_PAYLOAD

  inline static void create_payload(Data &data, Pivot &pivot) {
    std::vector<std::vector<float>> payload_buffer;

    auto size = 0;
    payload_buffer.emplace_back(PDigest::get_payload(data, pivot));
    size += payload_buffer.back().size();

    // allocate memory
    size += (payload_buffer.size() - 1);
    payload_t *payload = new stde::dynarray<float>(size);

    auto payload_index = 0;
    for (auto i = 1; i < payload_buffer.size(); ++i) {
      (*payload)[payload_index] = (float) payload_buffer[i].size();
      ++payload_index;
    }

    for (const auto &ctn : payload_buffer) {
      std::memcpy(&(*payload)[payload_index], &ctn[0], ctn.size() * sizeof(float));
      payload_index += ctn.size();
    }

    pivot.set_payload(payload);
  }

  inline static pivot_ctn *create_link(Data &data, bined_pivot_t &binned, const build_ctn &ctn, const link_ctn &links) {
    pivot_ctn *link = new pivot_ctn(ctn.size());
    std::memcpy(&(*link)[0], &ctn[0], ctn.size() * sizeof(Pivot));

#ifdef NDS_ENABLE_PAYLOAD
#ifdef NDS_SHARE_PAYLOAD
    for (auto &link_ctn : links) {
      share_payload(link_ctn->begin(), link_ctn->end(), link->begin(), link->end());
    }
#endif // NDS_SHARE_PAYLOAD

    for (auto &ptr : *link) {
      if (ptr.get_payload() == nullptr) {
        create_payload(data, ptr);
      }
    }
#endif // NDS_ENABLE_PAYLOAD
    return link;
  }

  inline static pivot_ctn *get_link(Data &data, bined_pivot_t &binned, const build_ctn &ctn, const link_ctn &links) {
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

  inline static
  void share(Data &data, bined_pivot_t &binned, const build_ctn &ctn, const link_ctn &links, link_ctn &share) {
    pivot_ctn *link = get_link(data, binned, ctn, links);

    binned.pivots = link;
    share.emplace_back(link);
  }

 private:
  pivot_ctn _root;
  std::vector<std::unique_ptr<Payload>> _payload;
  std::vector<std::unique_ptr<Dimension>> _dimension;
};
