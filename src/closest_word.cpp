// Given a table of edit costs and a lexicon, return the closest word
// in the lexicon for each word in the input.
//
// usage: closest_word [-i cost] [-d cost] [-s cost] costs lexicon < words > closest
//
// Ales Tamchyna <tamchyna@ufal.mff.cuni.cz>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <limits>
#include <getopt.h>
#include <mutex>
#include <omp.h>
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

string usage() {
  return "usage: closest_word [-c cores] [-n N] [-l max_length_difference] [-i cost] [-d cost] [-s cost] costs lexicon < words > closest";
}

int main(int argc, char **argv) {
  float insCost, delCost, subCost;
  insCost = delCost = subCost = 1;
  size_t closestN = 1;
  size_t maxLengthDiff = 0;
  size_t maxCores = 1;

  // command-line options
  int opt;
  extern char *optarg;
  extern int optind;
  while ((opt = getopt(argc, argv, "hi:d:s:n:l:c:")) != EOF) {
    switch (opt) {
      case 'n':
        closestN = lexical_cast<size_t>(optarg);
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
      case 'l':
        maxLengthDiff = lexical_cast<size_t>(optarg);
        break;
      case 'c':
        maxCores = lexical_cast<size_t>(optarg);
        break;
      case 'h':
        cerr << usage() << endl;
        return 0;
      default:
        die(usage());
    }
  }
  if (argc != optind + 2) die(usage());
  CostTable<uint32_t> costs(insCost, delCost, subCost);
  costs.load(argv[optind]);

  omp_set_num_threads(maxCores);
  cerr << "Using " << omp_get_max_threads() << " cores.\n";

  // load the lexicon
  cerr << "Loading lexicon from " << argv[optind + 1] << "...\n";
  vector<vector<uint32_t> > lexicon;
  ifstream lexIn(argv[optind + 1]);
  string line;
  size_t lineNum = 0;
  while (getline(lexIn, line)) {
    if (++lineNum % 10000 == 0) cerr << ".";
    lexicon.push_back(utf8_to_unsigned32(line));
  }
  
  // process the input
  cerr << "\nProcessing input...\n";
  lineNum = 0;
  while (getline(cin, line)) {
    if (++lineNum % 10000 == 0) cerr << ".";
    vector<uint32_t> decoded = utf8_to_unsigned32(line);
    FixedQueue<pair<float, vector<uint32_t> > > closest(closestN);
    mutex queueMutex;

    #pragma omp parallel for
    for (size_t idx = 0; idx < lexicon.size(); idx++) {
      const auto &lexWord = lexicon[idx];
      if (maxLengthDiff && abs(decoded.size() - lexWord.size()) > maxLengthDiff)
        continue;
      float dist = levenshtein(lexWord, decoded, costs);

      queueMutex.lock();
      closest.push({dist, lexWord});
      queueMutex.unlock();
    }

    auto queue = closest.getQueue();
    sort(queue.begin(), queue.end());
    for (size_t i = 0; i < closestN; i++) {
      cout << unsigned32_to_utf8(queue[i].second) << ":" << queue[i].first;
      cout << (i + 1 < closestN ? "\t" : "\n");
    }
  }

  cerr << "Done\n";

  return 0;
}
