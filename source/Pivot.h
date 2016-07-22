#pragma once

#include "stdafx.h"
#include "types.h"

class BinnedPivot;

class Pivot {
public:
   Pivot() = default;

   Pivot(uint32_t first, uint32_t second) : _pivot({ first, second }) { };

   Pivot(const Pivot&) = default;

   ~Pivot() = default;

   inline bool empty() const;
   inline uint32_t size() const;

   inline uint32_t front() const;
   inline uint32_t back() const;

   inline bool intersect_range(const Pivot& lhs, const Pivot& rhs) const;

   inline bool operator<(const Pivot& other) const;
   inline bool operator>(const Pivot& other) const;

   inline bool operator<=(const Pivot& other) const;
   inline bool operator>=(const Pivot& other) const;

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

using pivot_container = std::vector<Pivot*>;
using pivot_iterator = pivot_container::const_iterator;

using building_container = std::vector<Pivot>;
using building_iterator = building_container::const_iterator;

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

bool Pivot::intersect_range(const Pivot& lhs, const Pivot& rhs) const {
   return !(rhs.back() <= front() || lhs.front() >= back());
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
