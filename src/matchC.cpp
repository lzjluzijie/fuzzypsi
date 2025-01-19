#include "fuzzypsi.hpp"

task<> Fuzzy::matchC0(cp::Socket& chl, Matrix<u64>& data0) {
  u64 senderSize = data0.rows();
  u64 dim = data0.cols();
  u64 maxDcmLength = log2ceil(2 * radius + 1);
  co_await chl.send(senderSize);

  auto param = CuckooIndex<>::selectParams(senderSize, 40, 0, 3);
  u64 m = static_cast<u64>(senderSize * param.mBinScaler);

  u64 nTriples = (dim - 1 + hashLength) * (m * binSize);
  for (u64 k = 0; k < maxDcmLength; k++) {
    nTriples += min(maxDcmLength - 1, maxDcmLength - k) * (m * binSize * dim);
  }
  Triples triples(nTriples);
  triples.eagerGen0(chl);

  vector<block> hashIn(senderSize);
  vector<u64> preHash(dim);
  HASH hash{};
  for (u64 i = 0; i < senderSize; i++) {
    for (u64 k = 0; k < dim; k++) {
      preHash[k] = data0[i][k] - (data0[i][k] % (2 * radius));
    }
    SHA256(reinterpret_cast<const unsigned char*>(preHash.data()),
           dim * sizeof(u64), hash.data);
    hashIn[i] = *reinterpret_cast<block*>(hash.data);
  }

  CuckooIndex<> cuckoo;
  cuckoo.init(param);
  cuckoo.insert(hashIn);

  vector<block> hashes(m * binSize, AllOneBlock);
  Matrix<u64> compare0(m * binSize, dim);
  for (u64 i = 0; i < m; i++) {
    if (!cuckoo.mBins[i].isEmpty()) {
      for (u64 j = 0; j < binSize; j++) {
        hashes[i * binSize + j] = hashIn[cuckoo.mBins[i].idx()];;
        for (u64 k = 0; k < dim; k++) {
          u64 cur = data0[cuckoo.mBins[i].idx()][k];
          cur = cur % (2 * radius);
          compare0[i * binSize + j][k] = cur;
        }
      }
    }
  }

  BitVector res0;

  if (fake) {
    co_await fakeMatchC1(chl, compare0, hashes, res0);
  } else {
    co_await matchDcmC1(chl, maxDcmLength, compare0, hashes, res0, triples);
    co_await chl.send(res0);
  }

  co_await chl.flush();
}

task<> Fuzzy::matchC1(cp::Socket& chl, Matrix<u64>& data1) {
  u64 receiverSize = data1.rows();
  u64 dim = data1.cols();
  u64 maxDcmLength = log2ceil(2 * radius + 1);
  u64 senderSize = receiverSize;
  co_await chl.recv(senderSize);

  auto param = CuckooIndex<>::selectParams(senderSize, 40, 0, 3);
  u64 m = static_cast<u64>(senderSize * param.mBinScaler);

  u64 nTriples = (dim - 1 + hashLength) * (m * binSize);
  for (u64 k = 0; k < maxDcmLength; k++) {
    nTriples += min(maxDcmLength - 1, maxDcmLength - k) * (m * binSize * dim);
  }
  Triples triples(nTriples);
  triples.eagerGen1(chl);

  Matrix<block> hashIn(receiverSize, 1 << dim);
  vector<u64> curItem(dim);
  vector<u64> preHash(dim);
  HASH hash{};
  for (u64 i = 0; i < receiverSize; i++) {
    for (u64 k = 0; k < dim; k++) {
      curItem[k] = data1[i][k] - radius;
      curItem[k] = curItem[k] - (curItem[k] % (2 * radius));
    }

    for (u64 j = 0; j < (1ull << dim); j++) {
      for (u64 k = 0; k < dim; k++) {
        if (j & (1 << k)) {
          preHash[k] = curItem[k] + 2 * radius;
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
  vector<block> hashes(m * binSize);
  Matrix<u64> compare1(m * binSize, dim);
  for (u64 i = 0; i < m * binSize; i++) {
    for (u64 j = 0; j < dim; j++) {
      compare1[i][j] = -(2 * radius + 1);
    }
  }
  for (u64 i = 0; i < (receiverSize << dim); i++) {
    u64 dimId = i % (1 << dim);
    for (u64 j = 0; j < 3; j++) {
      u64 idx = locations(i, j);
      if (simpleHashes[idx].size() >= binSize) {
        cout << "bin is full: " << idx << endl;
        exit(1);
      }
      for (u64 k = 0; k < dim; k++) {
        if (dimId & (1 << k)) {
          u64 cur = data1[i >> dim][k] + radius;
          cur = cur % (2 * radius);
          compare1[idx * binSize + simpleHashes[idx].size()][k] = -cur;;
        } else {
          u64 cur = data1[i >> dim][k] - radius;
          cur = cur % (2 * radius);
          compare1[idx * binSize + simpleHashes[idx].size()][k] = cur;
        }
      }
      hashes[idx * binSize + simpleHashes[idx].size()] =
          hashIn[i >> dim][dimId];
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
    co_await fakeMatchC0(chl, compare1, hashes, res1);
  } else {
    co_await matchDcmC0(chl, maxDcmLength, compare1, hashes, res1, triples);
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
        // cout << "matched bin " << i << ", item " << j << ", input " <<
        //     simpleHashes[i][j] << endl;
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
