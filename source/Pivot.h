#pragma once

#include "stdafx.h"
#include "types.h"

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

	inline bool operator< (const Pivot& other) const;
	inline bool operator> (const Pivot& other) const;

   inline bool contains(const Pivot& other) const;
   inline bool endsWith(const Pivot& other) const;

   // implicit conversion
	inline operator const Pivot*() const;

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

bool Pivot::operator<(const Pivot& other) const {
	return back() < other.front();
}

bool Pivot::operator>(const Pivot& other) const {
	return back() > other.front();
}

bool Pivot::contains(const Pivot& other) const {
   return front() <= other.front() && back() >= other.back();
}

bool Pivot::endsWith(const Pivot& other) const {
   return back() == other.back();
}

Pivot::operator const Pivot*() const {
	return this;
}
