#pragma once

#include "stdafx.h"
#include "types.h"

class Pivot {
public:
	Pivot(uint32_t first, uint32_t second) : _pivot({ first, second }) { };

	Pivot(const Pivot&) = default;

	~Pivot() = default;

	inline bool empty() const;
	inline uint32_t size() const;

	inline uint32_t front() const;
	inline uint32_t back() const;

	inline bool operator< (const Pivot& other) const;
	inline bool operator> (const Pivot& other) const;

	// implicit conversion
	inline operator const Pivot*() const;

	inline bool endAfter(const Pivot& other) const;
	inline bool endBefore(const Pivot& other) const;

protected:
	std::array<uint32_t, 2> _pivot;
};

using pivot_container = std::vector<Pivot*>;
using pivot_iterator = pivot_container::const_iterator;

using building_container = std::vector<Pivot>;
using response_iterator = building_container::const_iterator;


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

Pivot::operator const Pivot*() const {
	return this;
}

bool Pivot::endAfter(const Pivot& other) const {
	return back() > other.front();
}

bool Pivot::endBefore(const Pivot& other) const {
	return back() <= other.front();
}