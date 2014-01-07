// Read tab-delimited strings from STDIN, output their character (default)
// or word (-w) edit distance. Input is *assumed* to be in UTF8.
//
// Ales Tamchyna <tamchyna@ufal.mff.cuni.cz>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <getopt.h>
#include "utf8.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>

#include "Util.hpp"
#include "Levenshtein.hpp"

using namespace std;
using namespace boost;
using namespace boost::algorithm;
using namespace Util;

vector<string> parseLine (string &line) {
  vector<string> cols;
  trim_if(line, is_any_of(" "));
  split(cols, line, is_any_of("\t"));
  if (cols.size() != 2) {
    die("Bad line format, expected 2 tab-delimited strings.\n"
        "Run with -h to get more information.");
  }
  return cols;
}

int main(int argc, char **argv) {
  bool wordEditDist = false;
  string costsFile;
  float insCost, delCost, subCost;
  insCost = delCost = subCost = 1;

  // command-line options
  int opt;
  extern char *optarg;
  while ((opt = getopt(argc, argv, "hwc:i:d:s:")) != EOF) {
    switch (opt) {
      case 'h':
        cerr << "Calculate edit distance between tab-delimited strings read"
          " from standard input (UTF-8 encoding is assumed).\n"
          "Options:\n"
          "  -w        Calculate the distance on words\n"
          "  -c <file> Read operation costs from file\n"
          "  -i <x>    Default cost of insertion\n"
          "  -d <x>    Default cost of deletion\n"
          "  -s <x>    Default cost of substition\n"
          "  -h        Print this message\n";
        return 0;
      case 'w':
        wordEditDist = true;
        break;
      case 'c':
        costsFile = optarg;
        break;
      case 's':
        subCost = lexical_cast<float>(optarg);
        break;
      case 'd':
        delCost = lexical_cast<float>(optarg);
        break;
      case 'i':
        insCost = lexical_cast<float>(optarg);
        break;
      default:
        die("Bad usage, run with -h to get a list of accepted options");
    }
  }

  string line;
  if (wordEditDist) {
    CostTable<vector<uint32_t> > costs(insCost, delCost, subCost);
    if (! costsFile.empty()) costs.load(costsFile);
    while (getline(cin, line)) {
      vector<string> cols = parseLine(line);
      vector<string> a, b;
      vector<vector<uint32_t> > a_uint32, b_uint32;
      split(a, cols[0], is_any_of(" "));
      split(b, cols[1], is_any_of(" "));
      for (const auto &word : a) a_uint32.push_back(utf8_to_unsigned32(word));
      for (const auto &word : b) b_uint32.push_back(utf8_to_unsigned32(word));
      cout << levenshtein(a_uint32, b_uint32, costs) << "\n";
    }
  } else {
    CostTable<uint32_t> costs(insCost, delCost, subCost);
    if (! costsFile.empty()) costs.load(costsFile);
    while (getline(cin, line)) {
      vector<string> cols = parseLine(line);
      cout << levenshtein(utf8_to_unsigned32(cols[0]), utf8_to_unsigned32(cols[1]), costs) << "\n";
    }
  }

  return 0;
}
