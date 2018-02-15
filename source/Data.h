#pragma once

#include "types.h"

class Schema;

class Data {
  struct DataElement {
    uint32_t hash, index;
    inline bool operator<(const DataElement &e1) const {
      return hash < e1.hash;
    }
  };

 public:
  Data(const Schema &schema);
  ~Data() = default;

  void sort(size_t fromIndex, size_t toIndex);
  void setHash(size_t id, uint32_t value);

  template<typename T>
  T *record(size_t id);
  inline float payload(uint32_t offset, size_t id);

  template<typename T>
  void prepareOffset(uint8_t offset);
  inline void preparePayload(uint8_t offset);

  inline uint32_t size() const {
    return _num_elts;
  }

 private:
  static inline bool comparator(const DataElement &bit0, const DataElement &bit1) {
    return bit0.hash < bit1.hash;
  }

  size_t _num_elts{0};

  std::vector<BinaryHeader> _headers;
  std::vector<std::string> _paths;

  std::vector<uint8_t> _data;
  std::vector<DataElement> _element;

  // [payload offset] -> data
  std::unordered_map<uint32_t, std::vector<float>> _payload;
};

template<typename T>
T *Data::record(size_t id) {
  return ((T *) &_data[_element[id].index * sizeof(T)]);
}

float Data::payload(uint32_t offset, size_t id) {
  return _payload[offset][_element[id].index];
}

template<typename T>
void Data::prepareOffset(uint8_t offset) {
  _data.clear();
  _data.resize(sizeof(T) * _num_elts);

  size_t curr_size = 0;
  for (auto i = 0; i < _paths.size(); ++i) {
    std::ifstream infile(_paths[i], std::ios::binary);

    infile.ignore(sizeof(BinaryHeader) + (_headers[i].records * offset));

    infile.read(reinterpret_cast<char *>(&_data[curr_size]), sizeof(T) * _headers[i].records);

    curr_size += sizeof(T) * _headers[i].records;

    infile.close();
  }
}

void Data::preparePayload(uint8_t offset) {
  if (_payload.find(offset) == _payload.end()) {

    _payload.emplace(offset, std::vector<float>(_num_elts));

    size_t curr_size = 0;
    for (auto i = 0; i < _paths.size(); ++i) {
      std::ifstream infile(_paths[i], std::ios::binary);

      infile.ignore(sizeof(BinaryHeader) + (_headers[i].records * offset));

      infile.read(reinterpret_cast<char *>(&_payload[offset][curr_size]), sizeof(float) * _headers[i].records);

      curr_size += _headers[i].records;

      infile.close();
    }
  }
}

