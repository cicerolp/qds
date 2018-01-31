#include "Data.h"

Data::Data(const std::string& path) : _path(path) {
  std::ifstream infile(_path, std::ios::binary);

  infile.read((char*)&_header, sizeof(BinaryHeader));

  infile.close();

  _element.resize(_header.records);
  for (auto i = 0; i < _element.size(); i++) _element[i].index = (uint32_t)i;
}

void Data::sort(size_t fromIndex, size_t toIndex) {
  std::sort(_element.begin() + fromIndex, _element.begin() + toIndex);
}

void Data::setHash(size_t id, uint32_t value) { _element[id].hash = value; }
