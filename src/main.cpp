#include "fuzzypsi.hpp"

void test() {};

int main(int argc, char** argv) {
  if (argc >= 2) {
    if (string(argv[1]) == "sim") {
      u64 senderSize = 1 << 11;
      if (argc >= 3) {
        senderSize = atoi(argv[2]);
      }
      u64 receiverSize = 1 << 11;
      if (argc >= 4) {
        receiverSize = atoi(argv[3]);
      }
      u64 dim = 5;
      if (argc >= 5) {
        dim = atoi(argv[4]);
      }
      u64 trails = 1 << 16;
      if (argc >= 6) {
        trails = atoi(argv[5]);
      }

      CuckooParam param = CuckooIndex<>::selectParams(senderSize, 40, 0, 3);
      u64 m = static_cast<u64>(senderSize * param.mBinScaler);
      cout << senderSize << " " << m << " " << param.mBinScaler << endl;

      vector<u64> maxMap =
          simulate_probability(receiverSize << dim, m, 3, trails);
      u64 count = 0;
      for (u64 i = 0; i < maxMap.size(); ++i) {
        if (maxMap[i] > 0) {
          cout << i << ": " << maxMap[i] << "," << endl;
          count += maxMap[i];
          if (count == trails) {
            break;
          }
        }
      }
    } else if (string(argv[1]) == "bench") {
      string protocol = "C";
      if (argc >= 3) {
        protocol = argv[2];
      }
      u64 senderSize = 1 << 11;
      if (argc >= 4) {
        senderSize = atoi(argv[3]);
      }
      u64 receiverSize = 1 << 11;
      if (argc >= 5) {
        receiverSize = atoi(argv[4]);
      }
      u64 dim = 5;
      if (argc >= 6) {
        dim = atoi(argv[5]);
      }
      u64 radius = 30;
      if (argc >= 7) {
        radius = atoi(argv[6]);
      }
      u64 binSize = 129;
      if (argc >= 8) {
        binSize = atoi(argv[7]);
      }
      bench(protocol, senderSize, receiverSize, dim, radius, binSize);
    } else {
      cout << "Unknown command: " << argv[1] << endl;
    }
  } else {
    test();
  }
}