#pragma once

#include "types.h"

class Data {
   struct DataElement {
      uint8_t hash;
      uint32_t index;
      
      bool operator<(const DataElement& e1) const {
         return hash < e1.hash;
      }
   };

public:
	Data(const std::string& path);
	~Data() = default;

   void sort(size_t fromIndex, size_t toIndex);
   void setHash(size_t id, uint8_t value);

   template<typename T>
   T record(size_t id, uint8_t offset);

   inline uint32_t size() const;

private:
   inline uint8_t getHash(size_t id) const;

   inline bool comparator(const DataElement& bit0, const DataElement& bit1) const;

   BinaryHeader _header;

   std::vector<uint8_t> _data;
   std::vector<DataElement> _element;
};

template<typename T>
T Data::record(size_t id, uint8_t offset) {
   return *((T*)&_data[(_element[id].index * sizeof(T)) + (_header.records * offset)]);
}

uint32_t Data::size() const {
   return _header.records;
}

uint8_t Data::getHash(size_t id) const {
   return _element[id].hash;
}

bool Data::comparator(const DataElement& bit0, const DataElement& bit1) const {   
   return bit0.hash < bit1.hash;
}

