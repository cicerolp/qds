#pragma once

#include "Schema.h"

class Data {
public:
	Data(const Schema& schema);
	virtual ~Data();

   void read(const std::string& path);

   void sort(size_t fromIndex, size_t toIndex);
   void setHash(size_t id, uint8_t value);

   template<typename T>
   T record(size_t id, uint8_t offset);

private:
   inline uint8_t getHash(size_t id) const;

   inline bool comparator(size_t bit0, size_t bit1) const;

   Schema _schema;
   BinaryHeader _header;

   std::vector<uint8_t> _data;

   std::vector<uint32_t> _index;
   std::vector<uint8_t> _hash;
};

template<typename T>
T Data::record(size_t id, uint8_t offset) {
   return *((T*)&_data[(_index[id] * _header.bytes) + offset]);
}

uint8_t Data::getHash(size_t id) const {
   return _hash[id];
}

bool Data::comparator(size_t bit0, size_t bit1) const {
   return getHash(bit0) < getHash(bit1);
}

