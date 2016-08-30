#pragma once

#include "stdafx.h"
#include "types.h"

class BinnedPivot;

class Pivot {
public:
   Pivot() = default;

   Pivot(uint32_t first, uint32_t second) : _pivot({ first, second }) { };

   Pivot(const Pivot& other) = default;
   Pivot(Pivot&& other) = default;
   Pivot& operator=(const Pivot& other) = default;
   Pivot& operator=(Pivot&& other) = default;

   inline bool empty() const;
   inline uint32_t size() const;

   inline uint32_t front() const;
   inline uint32_t back() const;

   inline void front(uint32_t value);
   inline void back(uint32_t value);

   inline bool operator==(const Pivot& other) const;

   inline bool operator<(const Pivot& other) const;
   inline bool operator>(const Pivot& other) const;

   inline bool operator<=(const Pivot& other) const;
   inline bool operator>=(const Pivot& other) const;

   inline bool ends_before(const Pivot& other) const;
   inline bool begins_after(const Pivot& other) const;

   inline bool contains(const Pivot& lhs, const Pivot& rhs) const;

   friend std::ostream& operator<<(std::ostream& stream, const Pivot& pivot) {
      stream << "[" << pivot._pivot[0] << "," << pivot._pivot[1] << "]";
      return stream;
   }

   // implicit conversion
   inline operator const Pivot*() const;
   inline operator std::string() const;
   operator BinnedPivot() const;

   static inline bool lower_bound_comp(const Pivot& lhs, const Pivot& rhs);
   static inline bool upper_bound_comp(const Pivot& lhs, const Pivot& rhs);

protected:
   std::array<uint32_t, 2> _pivot;
};

using pivot_ctn = stde::dynarray<Pivot>;
using pivot_it = pivot_ctn::const_iterator;

using link_ctn = std::vector<pivot_ctn*>;
using link_it = link_ctn::const_iterator;

using build_ctn = std::vector<Pivot>;
using build_it = build_ctn::const_iterator;

struct binned_t {
public:
   // shared (false) or proper (true) content
   bool proper {false};

   uint64_t value;  
   pivot_ctn* pivots;

   ~binned_t() {
      if (proper) delete pivots;
   }

   inline pivot_ctn& ptr() const {
      return *pivots;
   }

   inline static bool Comp(const binned_t* lhs, const binned_t* rhs) {
      return lhs->value < rhs->value;
   }
};

using binned_ctn = std::vector<const binned_t*>;
using binned_it = binned_ctn::const_iterator;

struct subset_t {
   subset_t() : option(DefaultCopy) { }
   CopyOption option;
   binned_ctn container;
};

using subset_container = std::vector<subset_t>;

bool Pivot::empty() const {
   return (back() - front()) == 0;
}

uint32_t Pivot::size() const {
   return back() - front();
}

uint32_t Pivot::front() const {
   return _pivot[0];
}

uint32_t Pivot::back() const {
   return _pivot[1];
}

void Pivot::front(uint32_t value) {
   _pivot[0] = value;
}

void Pivot::back(uint32_t value) {
   _pivot[1] = value;
}

bool Pivot::operator==(const Pivot& other) const {
   return _pivot[0] == other._pivot[0] && _pivot[1] == other._pivot[1];
}

bool Pivot::operator<(const Pivot& other) const {
   return front() < other.front();
}

bool Pivot::operator>(const Pivot& other) const {
   return back() > other.back();
}

bool Pivot::operator<=(const Pivot& other) const {
   return front() <= other.front();
}

bool Pivot::operator>=(const Pivot& other) const {
   return back() >= other.back();
}

bool Pivot::ends_before(const Pivot& other) const {
   return back() <= other.front();
}

bool Pivot::begins_after(const Pivot& other) const {
   return front() >= other.back();
}

bool Pivot::contains(const Pivot& lhs, const Pivot& rhs) const {
   return front() <= lhs.front() && back() >= rhs.back();
}

Pivot::operator const Pivot*() const {
   return this;
}

Pivot::operator std::string() const {
   return "[" + std::to_string(_pivot[0]) + "," + std::to_string(_pivot[1]) + "]";
}

bool Pivot::lower_bound_comp(const Pivot& lhs, const Pivot& rhs) {
   return lhs.front() < rhs.front();
}

bool Pivot::upper_bound_comp(const Pivot& lhs, const Pivot& rhs) {
   return lhs.back() <= rhs.front();
}
