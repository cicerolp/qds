#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "Schema.h"
#include "Dimension.h"

class NDS {
public:
   NDS(const Schema& schema);

   NDS(const NDS& that) = delete;
   NDS& operator=(NDS const&) = delete;

   std::string query(const Query& query, std::ofstream* telemetry);

   interval_t get_interval() const;

   inline uint32_t size() const {
      return _root[0].back();
   }

   inline Data* data() const {
      return _data_ptr.get();
   }

   template<typename Container>
   inline static void swap_and_clear(Container& lhs, Container& rhs) {
      lhs.swap(rhs);
      rhs.clear();
   }

   inline pivot_ctn* create_link(binned_t& binned, const build_ctn& container) {
      pivot_ctn* link = new pivot_ctn(container.size());
      std::memcpy(&(*link)[0], &container[0], container.size() * sizeof(Pivot));

      binned.proper = true;

      return link;
   }

   inline pivot_ctn* get_link(binned_t& binned, const build_ctn& container, const link_ctn& links) {
      pivot_ctn* link = nullptr;

      for (auto& ptr : links) {
         if (ptr->size() == container.size() && std::equal(container.begin(), container.end(), (*ptr).begin())) {
            link = ptr;
            break;
         }
      }

      if (link == nullptr) return create_link(binned, container);
      else return link;
   }

   inline void share(binned_t& binned, const build_ctn& container, const link_ctn& links, link_ctn& share) {
      pivot_ctn* link = get_link(binned, container, links);

      binned.pivots = link;
      share.emplace_back(link);
   }

private:
   pivot_ctn _root;
   std::unique_ptr<Data> _data_ptr;
   std::vector<std::pair<Dimension::Type, std::unique_ptr<Dimension>>> _dimension;
};
