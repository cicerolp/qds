#pragma once

#include "Data.h"
#include "Query.h"
#include "Pivot.h"
#include "Schema.h"
#include "Dimension.h"

class NDS {
public:
   NDS(const Schema& schema);
   ~NDS();

   NDS(const NDS& that) = delete;
   NDS& operator=(NDS const&) = delete;

   std::string query(const Query& query, std::ofstream* telemetry);

   interval_t get_interval() const;

   inline uint32_t size() const {
      return pivots[0]->front().back();
   }

   inline Data* data() const {
      return data_ptr.get();
   }

   template<typename Container>
   inline static void swap_and_clear(Container& lhs, Container& rhs) {
      lhs.swap(rhs);
      rhs.clear();
   }

   inline pivot_ctn* get_link(const build_ctn& container) {
      auto ptr = new pivot_ctn(container.size());
      pivots.emplace_back(ptr);
      std::memcpy(&(*ptr)[0], &container[0], container.size() * sizeof(Pivot));
      return ptr;
   }

   inline void share(binned_t& binned, const build_ctn& container, const link_ctn& links, link_ctn& share) {
      pivot_ctn* link = nullptr;
      for (auto& ptr : links) {
         if (ptr->size() == container.size() && std::equal(container.begin(), container.end(), (*ptr).begin())) {
            std::cout << "share" << std::endl;
            link = ptr;
            break;
         }
      }

      if (link == nullptr) link = get_link(container);

      binned.pivots = link;
      share.emplace_back(link);
   }

private:
   std::unique_ptr<Data> data_ptr;
   std::vector<pivot_ctn*> pivots;
   std::vector<std::pair<Dimension::Type, std::unique_ptr<Dimension>>> _dimension;
};
