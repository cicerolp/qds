#include "stdafx.h"
#include "Data.h"
#include "Schema.h"

Data::Data(const Schema &schema) : _paths(schema.files) {
  _num_elts = 0;
  for (auto &path : _paths) {
    std::ifstream infile(path, std::ios::binary);

    BinaryHeader header;
    infile.read((char *) &header, sizeof(BinaryHeader));

    _headers.emplace_back(header);

    _num_elts += header.records;

    infile.close();
  }

  _element.resize(_num_elts);
  for (auto i = 0; i < _element.size(); i++) {
    _element[i].index = (uint32_t) i;
  }
}

void Data::sort(size_t fromIndex, size_t toIndex) {
  gfx::timsort(_element.begin() + fromIndex, _element.begin() + toIndex);
}

void Data::setHash(size_t id, uint32_t value) {
  _element[id].hash = value;
}
