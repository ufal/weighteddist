// Estimate weights of Levenshtein edit operations using a parallel corpus of words.
// The weight of each edit operation is -log( #edit_applied/#all_edits)
// i.e. the inverse of its empirical log-probability.
//
// Usage: paste sourcewords.list targetwords.list | estimate_weights > weights.txt
//
// Ales Tamchyna <tamchyna@ufal.mff.cuni.cz>

#include <iostream>
#include <vector>
#include <deque>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "Util.hpp"

using namespace std;
using namespace Util;
using namespace boost;
using namespace boost::algorithm;

enum Operation {
  INS, DEL, SUB, NOP
};

template <class T>
struct Edit {
  Edit(Operation op, const T &from, const T &to) : op(op), from(from), to(to) {}

  Operation op;
  T from, to;
};

struct Cell {
  Cell() : dist(0), op(INS) {}
  Cell(int dist) : dist(dist), op(INS) {}
  Cell(int dist, Operation op) : dist(dist), op(op) {}

  int dist;
  Operation op;
};

bool operator<(const Cell &first, const Cell &second) {
  return first.dist < second.dist;
}

template <class T>
deque<Edit<typename T::value_type> > trackLevenshtein(const T& s1, const T& s2) {
  typedef typename T::value_type TVal;
  deque<Edit<TVal> > edits;

  const int len1 = s1.size(), len2 = s2.size();
  vector<vector<Cell> > d(len1 + 1, vector<Cell>(len2 + 1));

  d[0][0] = 0;
  for (int i = 1; i <= len1; ++i) d[i][0].dist = i;
  for (int i = 1; i <= len2; ++i) d[0][i].dist = i;

  for (int i = 1; i <= len1; ++i) {
    for (int j = 1; j <= len2; ++j) {
      d[i][j] = min(min(
            Cell(d[i - 1][j].dist + 1, DEL),
            Cell(d[i][j - 1].dist + 1, INS)),
          Cell(d[i - 1][j - 1].dist + (s1[i - 1] == s2[j - 1] ? 0 : 1),
            s1[i - 1] == s2[j - 1] ? NOP : SUB));
    }
  }

  int i = len1;
  int j = len2;

  while (i > 0 && j > 0) {
    edits.push_front(Edit<TVal>(d[i][j].op, s1[i-1], s2[j-1]));
    switch (d[i][j].op) {
      case INS:
        j--;
        break;
      case DEL:
        i--; 
        break;
      case SUB:
      case NOP:
        i--; j--;
        break;
    }
  }

  return edits;
}

int main(int argc, char **argv) {
  string line;
  unordered_map<string, size_t> editCounts;
  size_t totalEdits = 0;
  while (getline(cin, line)) {
    vector<string> cols;
    trim_if(line, is_any_of(" "));
    split(cols, line, is_any_of("\t"));
    if (cols.size() != 2) die("Bad format on the following line:\n" + line + "\n");
    const auto &edits = trackLevenshtein(utf8_to_unsigned32(cols[0]), utf8_to_unsigned32(cols[1]));
    for (const auto &edit : edits) {
      string editStringRep;
      string from = unsigned32_to_utf8(vector<uint32_t>(1, edit.from));
      string to   = unsigned32_to_utf8(vector<uint32_t>(1, edit.to));
      if (edit.op == NOP) continue;
      switch (edit.op) {
        case INS:
          editStringRep = "i\t" + to;
          break;
        case DEL:
          editStringRep = "d\t" + from;
          break;
        case SUB:
          editStringRep = "s\t" + from + "\t" + to;
          break;
        default:
          break;
      }
      editCounts[editStringRep]++;
      totalEdits++;
    }
  }

  for (const auto &edit: editCounts) {
    cout << edit.first << "\t" << 1.0 / edit.second << "\n";
  }
  return 0;
}
