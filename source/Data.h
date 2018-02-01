#pragma once

#include "types.h"

class Data {
  struct DataElement {
    uint32_t hash, index;
    inline bool operator<(const DataElement &e1) const {
      return hash < e1.hash;
    }
  };

 public:
  Data(const std::string &path);
  ~Data() = default;

  void sort(size_t fromIndex, size_t toIndex);
  void setHash(size_t id, uint32_t value);

  inline bool has_payload() const {
    return _payload.size() != 0;
  }

  template<typename T>
  T *record(size_t id);
  template<typename T>
  void prepareOffset(uint8_t offset);

  inline float payload(uint32_t offset, size_t id);
  inline void preparePayload(uint8_t offset);

  inline uint32_t size() const;

 private:
  static inline bool comparator(const DataElement &bit0,
                                const DataElement &bit1) {
    return bit0.hash < bit1.hash;
  }

  BinaryHeader _header;
  std::string _path;

  std::vector<uint8_t> _data;
  std::vector<DataElement> _element;

  // [payload offset] -> data
  std::unordered_map<uint32_t, std::vector<float>> _payload;
};

template<typename T>
T *Data::record(size_t id) {
  return ((T *) &_data[_element[id].index * sizeof(T)]);
}

template<typename T>
void Data::prepareOffset(uint8_t offset) {
  _data.clear();

  std::ifstream infile(_path, std::ios::binary);

  infile.ignore(sizeof(BinaryHeader) + (_header.records * offset));

  _data.resize(sizeof(T) * _header.records);
  infile.read(reinterpret_cast<char *>(&_data[0]), sizeof(T) * _header.records);

  infile.close();
}

float Data::payload(uint32_t offset, size_t id) {
  return _payload[offset][_element[id].index];
}

void Data::preparePayload(uint8_t offset) {
  std::ifstream infile(_path, std::ios::binary);

  if (_payload.find(offset) == _payload.end()) {
    infile.ignore(sizeof(BinaryHeader) + (_header.records * offset));

    _payload.emplace(offset, std::vector<float>(_header.records));

    infile.read(reinterpret_cast<char *>(&_payload[offset][0]), sizeof(float) * _header.records);
  }

  infile.close();
}

uint32_t Data::size() const { return _header.records; }
