#include <vector>
#include <iostream>
#include "Util.hpp"

using namespace std;
using namespace Util;

int main(int argc, char **argv) {
  FixedQueue<int> queue(5);
  for (int i = 0; i <= 10; i++) queue.push(i);
  for (auto i : queue.getQueue()) cout << i << "\n";
}
