#include "stdafx.h"
#include "Data.h"

Data::Data(const Schema& schema) : _schema(schema) {

}

Data::~Data() {

}

void Data::read(const std::string& path) {
   
   std::ifstream infile(path, std::ios::binary);
   infile.unsetf(std::ios_base::skipws);

   infile.read((char*)&_header, sizeof BinaryHeader);

   _data.resize(_header.bytes * _header.records);
   
   infile.read((char*)&_data, _header.bytes * _header.records);

   infile.close();

   _hash.resize(_header.records, 0);
   _index.resize(_header.records);

   for (size_t i = 0; i < _index.size(); i++) _index[i] = (uint32_t)i;

}

void Data::sort(size_t fromIndex, size_t toIndex) {
   const auto functor = std::bind(&Data::comparator, this, std::placeholders::_1, std::placeholders::_2);
   std::sort(_index.begin() + fromIndex, _index.begin() + toIndex, functor);
}

void Data::setHash(size_t id, uint8_t value) {
   _hash[_index[id]] = value;
}
