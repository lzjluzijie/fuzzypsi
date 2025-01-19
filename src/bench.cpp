#include "fuzzypsi.hpp"

void benchTriple(u64& duration, u64 n) {
  BitVector a0(n), b0(n), c0(n), a1(n), b1(n), c1(n);
  auto chls = cp::LocalAsyncSocket::makePair();
  thread tm0 = thread([&]() {
    //
    sync_wait(triple0(chls[0], a0, b0, c0));
  });
  thread tm1 = thread([&]() {
    auto start = std::chrono::high_resolution_clock::now();
    sync_wait(triple1(chls[1], a1, b1, c1));
    auto end = std::chrono::high_resolution_clock::now();
    duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
        .count();
  });
  tm0.join();
  tm1.join();
}

void benchIM(u64 n, u64 length, u64 radius) {
  u64 height = log2ceil(2 * radius + 1);
  u64 numTriples = 0;
  for (u64 k = 0; k < height; k++) {
    numTriples += min(length - 1, length - k) * 2 * n;
  }

  vector<u64> left(n), right(n), nums(n);
  for (u64 i = 0; i < n; i++) {
    left[i] = i;
    right[i] = i + 2 * radius;
    nums[i] = i + radius;
  }
  for (u64 i = 8; i < 16; i++) {
    nums[i] = i - 1 - (i / 2);
  }

  BitVector res0(n), res1(n);
  Triples triples0(numTriples), triples1(numTriples);
  auto chls = cp::LocalAsyncSocket::makePair();
  thread tm0 = thread([&]() {
    sync_wait(triples0.gen0(chls[0]));
    sync_wait(match1dB0(chls[0], length, height, left, right, res0, triples0));
  });
  thread tm1 = thread([&]() {
    Timer timer;
    timer.setTimePoint("start");
    sync_wait(triples1.gen1(chls[1]));
    timer.setTimePoint("triple");
    u64 tripleComm = chls[1].bytesSent() + chls[1].bytesReceived();
    sync_wait(match1dB1(chls[1], length, height, nums, res1, triples1));
    timer.setTimePoint("end");

    cout << timer << endl;
    cout << "Triple communication: " << tripleComm << " bytes, "
        << tripleComm / 1024 << " KiB, "
        << tripleComm / 1024 / 1024 << " MiB" << endl;
    u64 totalComm = chls[1].bytesSent() + chls[1].bytesReceived();
    cout << "Total communication: " << totalComm << " bytes, "
        << totalComm / 1024 << " KiB, "
        << totalComm / 1024 / 1024 << " MiB" << endl;
  });
  tm0.join();
  tm1.join();

  u64 matches = 0;
  for (u64 i = 0; i < n; i++) {
    if (res0[i] ^ res1[i]) {
      matches++;
    }
  }
  cout << "matches: " << matches << endl;
}

void benchDcm(u64 n, u64 length, u64 dim, u64 radius) {
  u64 height = log2ceil(2 * radius + 1);
  u64 nTriples = (dim - 1) * (n * dim);
  for (u64 k = 0; k < height; k++) {
    nTriples += min(length - 1, length - k) * 2 * (n * dim);
  }

  Matrix<u64> left(n, dim), right(n, dim), nums(n, dim);
  for (u64 i = 0; i < n; i++) {
    for (u64 k = 0; k < dim; k++) {
      left[i][k] = i;
      right[i][k] = i + 2 * radius;
      nums[i][k] = i + radius;
    }
  }
  for (u64 i = 8; i < 16; i++) {
    for (u64 k = 0; k < dim; k++) {
      nums[i][k] = i - 1 - (i / 2);
    }
  }

  BitVector res0(n), res1(n);
  Triples triples0(nTriples), triples1(nTriples);
  auto chls = cp::LocalAsyncSocket::makePair();
  thread tm0 = thread([&]() {
    sync_wait(triples0.gen0(chls[0]));
    sync_wait(matchDcmA0(chls[0], length, height, left, right, res0, triples0));
  });
  thread tm1 = thread([&]() {
    Timer timer;
    timer.setTimePoint("start");
    sync_wait(triples1.gen1(chls[1]));
    timer.setTimePoint("triple");
    u64 tripleComm = chls[1].bytesSent() + chls[1].bytesReceived();
    sync_wait(matchDcmA1(chls[1], length, height, nums, res1, triples1));
    timer.setTimePoint("end");

    cout << timer << endl;
    cout << "Triple communication: " << tripleComm << " bytes, "
        << tripleComm / 1024 << " KiB, "
        << tripleComm / 1024 / 1024 << " MiB" << endl;
    u64 totalComm = chls[1].bytesSent() + chls[1].bytesReceived();
    cout << "Total communication: " << totalComm << " bytes, "
        << totalComm / 1024 << " KiB, "
        << totalComm / 1024 / 1024 << " MiB" << endl;
  });
  tm0.join();
  tm1.join();

  u64 matches = 0;
  for (u64 i = 0; i < n; i++) {
    if (res0[i] ^ res1[i]) {
      matches++;
    }
  }
  cout << "matches: " << matches << endl;
}

void Fuzzy::bench(string protocol, u64& duration, u64 senderSize,
                  u64 receiverSize) {
  auto data0 = genDataV0(senderSize, dim, radius * 4 * 2 + 1, 10000);
  auto data1 = genDataV0(receiverSize, dim, radius * 4 * 2 + 1, 10029);

  for (u64 i = 0; i < 8; i++) {
    for (u64 k = 0; k < dim; k++) {
      data1[8 + i][k] = i * radius * 4 * 2 + 1;
    }
  }

  auto chls = cp::LocalAsyncSocket::makePair();
  thread tm0 = thread([&]() {
    if (protocol == "A") {
      macoro::sync_wait(matchA0(chls[0], data0));
    } else if (protocol == "B") {
      macoro::sync_wait(matchB0(chls[0], data0));
    } else if (protocol == "C") {
      macoro::sync_wait(matchC0(chls[0], data0));
    }
  });
  thread tm1 = thread([&]() {
    auto start = std::chrono::high_resolution_clock::now();
    if (protocol == "A") {
      macoro::sync_wait(matchA1(chls[1], data1));
    } else if (protocol == "B") {
      macoro::sync_wait(matchB1(chls[1], data1));
    } else if (protocol == "C") {
      macoro::sync_wait(matchC1(chls[1], data1));
    }
    auto end = std::chrono::high_resolution_clock::now();
    duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
        .count();
  });
  tm0.join();
  tm1.join();
}

void print(vector<u64> durations) {
  for (u64 i = 0; i < durations.size(); i++) {
    cout << durations[i] << " ";
  }
  sort(durations.begin(), durations.end());
  cout << "median: " << durations[durations.size() / 2] << endl;
}

void bench(string protocol, u64 senderSize, u64 receiverSize, u64 dim,
           u64 radius, u64 binSize) {
  Fuzzy fuzzy(32, radius, dim, binSize);

  u64 trails = 5;
  vector<u64> durations(trails);
  for (u64 i = 0; i < trails; i++) {
    if (protocol == "IM") {
      benchIM(senderSize, dim, radius);
    }
    if (protocol == "Dcm") {
      benchDcm(senderSize, receiverSize, dim, radius);
    } else {
      fuzzy.bench(protocol, durations[i], senderSize, receiverSize);
    }
    sleep(5);
  }
  print(durations);
  cout << "\n\n" << endl;
}