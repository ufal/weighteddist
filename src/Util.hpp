#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <exception>
#include <stdexcept>
#include <cmath>
#include <random>
#include <queue>
#include <functional>
#include "utf8.h"

#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>

// C++11 doesn't know how to hash pairs for some reason.
// OK, let's adapt boost::hash_range for this
namespace std {

template <class T>
struct hash<pair<T, T> > {
public:
  size_t operator()(const pair<T, T> &x) const {
    size_t h = 0;
    boost::hash_combine(h, x.first);
    boost::hash_combine(h, x.second);
    return h;
  }
};

// and vectors too
template <class T>
struct hash<vector<T> > {
public:
  size_t operator()(const vector<T> &x) const {
    return boost::hash_range(x.begin(), x.end());
  }
};

} // namespace std

namespace Util {

// fixed-size priority queue, intended for short queues
// (an unsorted vector is used internally)
//
// Behaves as a min-queue by default, a custom comparator
// can be used (e.g. std::greater<T> for a max-queue).
template<class T, class Comp = std::less<T> >
class FixedQueue {
public:
  FixedQueue(size_t maxSize) : maxSize_(maxSize)
  {}

  void push(const T &what) {
    if (queue_.size() < maxSize_) {
      queue_.push_back(what);
    } else {
      auto it = std::max_element(queue_.begin(), queue_.end(), comparator_);
      if (comparator_(what, *it)) *it = what; // replace the largest element
    }
  }

  const T &top() const {
    return *std::min_element(queue_.begin(), queue_.end(), comparator_);
  }

  void pop() {
    queue_.erase(std::min_element(queue_.begin(), queue_.end(), comparator_));
  }

  const std::vector<T> &getQueue() const {
    return queue_;
  }

private:
  size_t maxSize_;
  std::vector<T> queue_;
  Comp comparator_;
};

std::vector<uint32_t> utf8_to_unsigned32(const std::string& str) {
  std::vector<uint32_t> out;
  utf8::utf8to32(str.begin(), str.end(), std::back_inserter(out));
  return out;
}

std::string unsigned32_to_utf8(const std::vector<uint32_t> &str) {
  std::string out;
  utf8::utf32to8(str.begin(), str.end(), std::back_inserter(out));
  return out;
}

inline void die(const std::string &msg = "") {
  std::cerr << msg << "\n";
  exit(1);
}

} // namespace Util
