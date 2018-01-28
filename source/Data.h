#pragma once

#include "types.h"

class Data {
  struct DataElement {
    uint32_t hash, index;
    inline bool operator<(const DataElement& e1) const {
      return hash < e1.hash;
    }
  };

 public:
  Data(const std::string& path);
  ~Data() = default;

  void sort(size_t fromIndex, size_t toIndex);
  void setHash(size_t id, uint32_t value);

  inline bool has_payload() const {
    return _payload.size() != 0;
  }

  template <typename T>
  T* record(size_t id);
  template <typename T>
  void prepareOffset(uint8_t offset);

  template <typename T>
  T* payload(size_t id);
  template <typename T>
  void preparePayloadOffset(uint8_t offset);

  void dispose();

  inline uint32_t size() const;

 private:
  static inline bool comparator(const DataElement& bit0,
                                const DataElement& bit1) {
    return bit0.hash < bit1.hash;
  }

  BinaryHeader _header;
  std::string _path;

  std::vector<uint8_t> _data;
  std::vector<uint8_t> _payload;
  std::vector<DataElement> _element;
};

template <typename T>
T* Data::record(size_t id) {
  return ((T*)&_data[_element[id].index * sizeof(T)]);
}

template <typename T>
void Data::prepareOffset(uint8_t offset) {
  std::ifstream infile(_path, std::ios::binary);

  infile.ignore(sizeof(BinaryHeader) + (_header.records * offset));

  _data.resize(sizeof(T) * _header.records);
  infile.read(reinterpret_cast<char*>(&_data[0]), sizeof(T) * _header.records);

  infile.close();
}

template <typename T>
T* Data::payload(size_t id) {
  return ((T*)&_payload[_element[id].index * sizeof(T)]);
}

template <typename T>
void Data::preparePayloadOffset(uint8_t offset) {
  std::ifstream infile(_path, std::ios::binary);

  infile.ignore(sizeof(BinaryHeader) + (_header.records * offset));

  _payload.resize(sizeof(T) * _header.records);
  infile.read(reinterpret_cast<char*>(&_payload[0]), sizeof(T) * _header.records);

  infile.close();
}

uint32_t Data::size() const { return _header.records; }
