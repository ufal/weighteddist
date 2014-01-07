#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "Util.hpp"

// decode string to internal representation, specialized for UTF-8 strings and
// single-character UTF-8 strings (which are converted to uint32_t)
template<class T>
inline T decode(const std::string &str) {
  // implicit type conversion, not used in this program
  return str;
}

template<>
inline uint32_t decode(const std::string &str) {
  // utf-8 characters
  std::vector<uint32_t> chars = Util::utf8_to_unsigned32(str);
  if (chars.size() != 1) 
    throw std::runtime_error("expected only one letter, found: " + str);
  return chars[0];
}

template<>
inline std::vector<uint32_t> decode(const std::string &str) {
  return Util::utf8_to_unsigned32(str);
}

template<class T>
class CostTable {
public:
  CostTable(float insCost = 1.0, float delCost = 1.0, float subCost = 1.0) :
    insCost_(insCost), delCost_(delCost), subCost_(subCost)
  {}

  void load(const std::string &file) {
    std::ifstream in(file.c_str());
    std::string line;
    while (std::getline(in, line)) {
      std::vector<std::string> cols;
      boost::algorithm::split(cols, line, boost::algorithm::is_any_of("\t"));    
      if (cols.empty()) throw std::runtime_error("Empty line in costs table");
      switch (cols[0][0]) {
        case 's':
          if (cols.size() != 4) 
            throw std::runtime_error("Bad number of arguments for substition");
          subTable_[{decode<T>(cols[1]), decode<T>(cols[2])}] = boost::lexical_cast<float>(cols[3]);
          break;
        case 'd':
          if (cols.size() != 3) 
            throw std::runtime_error("Bad number of arguments for deletion");
          delTable_[decode<T>(cols[1])] = boost::lexical_cast<float>(cols[2]);
          break;
        case 'i':
          if (cols.size() != 3) 
            throw std::runtime_error("Bad number of arguments for insertion");
          insTable_[decode<T>(cols[1])] = boost::lexical_cast<float>(cols[2]);
          break;
        default:
          throw std::runtime_error("Unrecognized operation type: " + cols[0]);
      }
    }
  }

  float del(T c) const {
    auto it = delTable_.find(c);
    if (it == delTable_.end())
      return delCost_;
    else
      return it->second;
  }

  float ins(T c) const {
    auto it = insTable_.find(c);
    if (it == insTable_.end())
      return insCost_;
    else
      return it->second;
  }

  float sub(T a, T b) const {
    if (a == b) return 0;

    auto it = subTable_.find({a, b});
    if (it == subTable_.end())
      return subCost_;
    else
      return it->second;
  }

private:
  float insCost_, delCost_, subCost_;
  std::unordered_map<T, float> insTable_, delTable_;
  std::unordered_map<std::pair<T, T>, float> subTable_;
};

template<class T>
float levenshtein(const T& s1, const T& s2, const CostTable<typename T::value_type> &costs) {
  const int len1 = s1.size(), len2 = s2.size();
  std::vector<std::vector<float> > d(len1 + 1, std::vector<float>(len2 + 1));

  d[0][0] = 0;
  float total = 0;
  for (int i = 1; i <= len1; ++i) {
    d[i][0] = costs.del(s1[i - 1]) + total;
    total = d[i][0];
  }

  total = 0;
  for (int i = 1; i <= len2; ++i) {
    d[0][i] = costs.ins(s2[i - 1]) + total;
    total = d[0][i];
  }

  for (int i = 1; i <= len1; ++i) {
    for (int j = 1; j <= len2; ++j) {
      d[i][j] = std::min(std::min(
            d[i - 1][j] + costs.del(s1[i - 1]),
            d[i][j - 1] + costs.ins(s2[j - 1])),
          d[i - 1][j - 1] + costs.sub(s1[i - 1], s2[j - 1]));
//      cerr << d[i][j] << " ";
    }
//    cerr << "\n";
  }

  return d[len1][len2];
}
