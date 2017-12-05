/*
 * DataPacket.h
 *
 *  Created on: 2017年12月4日
 *      Author: xueda
 */

#ifndef INCLUDE_DATAPACKET_H_
#define INCLUDE_DATAPACKET_H_

#include <mutex>

namespace network {

#define MAXSIZE 4096

class DataPacket {
 public:
  DataPacket()
      : data_(nullptr),
        capacity_(0),
        length_(0) {
    data_ = new unsigned char[MAXSIZE / 4];
    capacity_ += MAXSIZE / 4;
  }
  ~DataPacket() {
    if (data_ != nullptr) {
      delete[] data_;
    }
  }

  const int capacity() const {
    return capacity_;
  }
  const int length() const {
    return length_;
  }
  void CopyFromArray(const unsigned char *src, int size) {
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK(size <= MAXSIZE);
    CHECK_NOTNULL(data_);
    if (capacity_ < MAXSIZE) {
      delete[] data_;
      data_ = new unsigned char[size];
      memcpy(data_, src, size);
      capacity_ = size;
      length_ = size;
    }
    data_ = new unsigned char[size];
    memcpy(data_, src, size);
  }
  void CopyToArray(unsigned char *dst, int size) {
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_NOTNULL(data_);
    memcpy(dst, data_, size);
  }
  void PushBack(const unsigned char *src, int size) {
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK(size <= MAXSIZE);
    CHECK_NOTNULL(data_);
    if (capacity_ - length_ < size) {
      unsigned char *buffer = new unsigned char[capacity_ + size];
      memcpy(buffer, data_, length_);
      delete[] data_;
      memcpy(buffer + length_, src, size);
      length_ += size;
      data_ = buffer;
    }
  }

  std::mutex& mutex() const {
    return mutex_;
  }

 private:
  mutable std::mutex mutex_;
  unsigned char *data_;
  int capacity_;
  int length_;
};

#undef MAXSIZE

}  // namespace network

#endif /* INCLUDE_DATAPACKET_H_ */