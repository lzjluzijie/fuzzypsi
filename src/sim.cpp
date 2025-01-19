#include "fuzzypsi.hpp"

vector<u64> simulate_probability(u64 n, u64 m, u64 hash_functions, u64 trials) {
  u64 limit = 2025;
  vector<u64> maxMap(limit, 0);
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> dis(0, m - 1);

  for (u64 t = 0; t < trials; ++t) {
    vector<u64> hash_buckets(m, 0);
    for (u64 h = 0; h < hash_functions; ++h) {
      for (u64 i = 0; i < n; ++i) {
        u64 hash_value = dis(gen);
        hash_buckets[hash_value]++;
      }
    }

    u64 max_count = *max_element(hash_buckets.begin(), hash_buckets.end());
    if (max_count < limit) {
      maxMap[max_count]++;
    } else {
      cout << "Oops, that's too much: " << max_count << endl;
    }
  }

  return maxMap;
}
