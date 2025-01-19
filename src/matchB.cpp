#include "fuzzypsi.hpp"

task<> Fuzzy::matchB0(cp::Socket& chl, Matrix<u64>& data0) {
  u64 senderSize = data0.rows();
  u64 dim = data0.cols();
  co_await chl.send(senderSize);

  auto param = CuckooIndex<>::selectParams(senderSize, 40, 0, 3);
  u64 m = static_cast<u64>(senderSize * param.mBinScaler);

  u64 nTriples = (dim - 1) * (m * binSize);
  for (u64 k = 0; k < height; k++) {
    nTriples += min(maxLength - 1, maxLength - k) * 2 * (m * binSize * dim);
  }
  Triples triples(nTriples);
  triples.eagerGen0(chl);

  vector<block> hashIn(senderSize);
  vector<u64> preHash(dim);
  HASH hash{};
  for (u64 i = 0; i < senderSize; i++) {
    for (u64 k = 0; k < dim; k++) {
      preHash[k] = data0[i][k] - radius;
      preHash[k] = preHash[k] - (preHash[k] % (2 * radius));
    }
    SHA256(reinterpret_cast<const unsigned char*>(preHash.data()),
           dim * sizeof(u64), hash.data);
    hashIn[i] = *reinterpret_cast<block*>(hash.data);
  }

  CuckooIndex<> cuckoo;
  cuckoo.init(param);
  cuckoo.insert(hashIn);

  Matrix<u64> compare0(m * binSize, dim);
  memset(compare0.data(), -1, compare0.size() * sizeof(u64));
  for (u64 i = 0; i < m; i++) {
    if (!cuckoo.mBins[i].isEmpty()) {
      // cout << "i: " << i << ", idx: " << cuckoo.mBins[i].idx() <<
      //     ", hash: " << cuckoo.mBins[i].hashIdx() << endl;
      for (u64 j = 0; j < binSize; j++) {
        for (u64 k = 0; k < dim; k++) {
          compare0[i * binSize + j][k] = data0[cuckoo.mBins[i].idx()][k];
        }
      }
    }
  }
  BitVector res0;

  if (fake) {
    co_await fakeMatch0(chl, compare0, radius, res0);
  } else {
    Matrix<u64> left(m * binSize, dim);
    Matrix<u64> right(m * binSize, dim);
    for (u64 i = 0; i < m * binSize; i++) {
      for (u64 j = 0; j < dim; j++) {
        left[i][j] = compare0[i][j] - radius;
        right[i][j] = compare0[i][j] + radius;
      }
    }
    co_await matchDcmA0(chl, maxLength, height, left, right, res0, triples);
    co_await chl.send(res0);
  }

  co_await chl.flush();
}

task<> Fuzzy::matchB1(cp::Socket& chl, Matrix<u64>& data1) {
  u64 receiverSize = data1.rows();
  u64 dim = data1.cols();
  u64 senderSize = receiverSize;
  co_await chl.recv(senderSize);

  auto param = CuckooIndex<>::selectParams(senderSize, 40, 0, 3);
  u64 m = static_cast<u64>(senderSize * param.mBinScaler);

  u64 nTriples = (dim - 1) * (m * binSize);
  for (u64 k = 0; k < height; k++) {
    nTriples += min(maxLength - 1, maxLength - k) * 2 * (m * binSize * dim);
  }
  Triples triples(nTriples);
  triples.eagerGen1(chl);

  Matrix<block> hashIn(receiverSize, 1 << dim);
  vector<u64> curItem(dim);
  vector<u64> preHash(dim);
  HASH hash{};
  for (u64 i = 0; i < receiverSize; i++) {
    for (u64 k = 0; k < dim; k++) {
      curItem[k] = data1[i][k] - (data1[i][k] % (2 * radius));
    }

    for (u64 j = 0; j < (1ull << dim); j++) {
      for (u64 k = 0; k < dim; k++) {
        if (j & (1 << k)) {
          preHash[k] = curItem[k] - 2 * radius;
        } else {
          preHash[k] = curItem[k];
        }
      }
      SHA256(reinterpret_cast<const unsigned char*>(preHash.data()),
             dim * sizeof(u64), hash.data);
      hashIn[i][j] = *reinterpret_cast<block*>(hash.data);
    }
  }

  Matrix<u32> locations(receiverSize << dim, 3);
  CuckooIndex<> cuckoo;
  cuckoo.init(param);
  cuckoo.computeLocations(hashIn, locations);

  u64 largestBin = 0;
  vector<vector<u64> > simpleHashes(m);
  Matrix<u64> compare1(m * binSize, dim);
  memset(compare1.data(), -1, compare1.size() * sizeof(u64));
  for (u64 i = 0; i < (receiverSize << dim); i++) {
    for (u64 j = 0; j < 3; j++) {
      u64 idx = locations(i, j);
      if (simpleHashes[idx].size() >= binSize) {
        cout << "bin is full: " << idx << endl;
        exit(1);
      }
      for (u64 k = 0; k < dim; k++) {
        compare1[idx * binSize + simpleHashes[idx].size()][k] =
            data1[i >> dim][k];
      }
      simpleHashes[idx].push_back(i >> dim);

      largestBin = largestBin > simpleHashes[idx].size()
                     ? largestBin
                     : simpleHashes[idx].size();
    }
  }

  cout << "largest bin size: " << largestBin << endl;

  BitVector res1;
  BitVector res0(m * binSize);

  if (fake) {
    co_await fakeMatch1(chl, compare1, radius, res1);
  } else {
    co_await matchDcmA1(chl, maxLength, height, compare1, res1, triples);
    co_await chl.recv(res0);
  }

  u64 matches = 0;
  for (u64 i = 0; i < m; i++) {
    for (u64 j = 0; j < binSize; j++) {
      if (fake && res1[i * binSize + j] == 1) {
        matches++;
        break;
      }
      if (!fake && (res0[i * binSize + j] ^ res1[i * binSize + j])) {
        matches++;
        break;
      }
    }
  }

  cout << "matches: " << matches << endl;
  co_await chl.flush();

  u64 totalComm = chl.bytesSent() + chl.bytesReceived();
  cout << "Total communication: " << totalComm << " bytes, "
      << totalComm / 1024 << " KiB, "
      << totalComm / 1024 / 1024 << " MiB" << endl;
}
