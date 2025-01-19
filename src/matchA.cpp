#include "fuzzypsi.hpp"

task<> Fuzzy::matchA0(cp::Socket& chl, Matrix<u64>& data0) {
  u64 senderSize = data0.rows();
  u64 receiverSize = senderSize;
  u64 dim = data0.cols();
  co_await chl.send(senderSize);
  co_await chl.recv(receiverSize);

  Matrix<u64> left(senderSize * receiverSize, dim);
  Matrix<u64> right(senderSize * receiverSize, dim);
  for (u64 i = 0; i < senderSize; i++) {
    for (u64 j = 0; j < receiverSize; j++) {
      for (u64 k = 0; k < dim; k++) {
        left[i * receiverSize + j][k] = data0[i][k] - radius;
        right[i * receiverSize + j][k] = data0[i][k] + radius;
      }
    }
  }

  BitVector res0(senderSize);
  BitVector tmp0;
  if (fake) {
    co_await fakeMatch0(chl, data0, radius, tmp0);
  } else {
    co_await matchDcmA0(chl, maxLength, height, left, right, tmp0);
  }
  for (u64 j = 0; j < receiverSize; j++) {
    for (u64 i = 0; i < senderSize; i++) {
      res0[j] ^= tmp0[i * receiverSize + j];
    }
  }
  co_await chl.send(res0);

  co_await chl.flush();
}

task<> Fuzzy::matchA1(cp::Socket& chl, Matrix<u64>& data1) {
  u64 receiverSize = data1.rows();
  u64 dim = data1.cols();
  u64 senderSize = receiverSize;
  co_await chl.recv(senderSize);
  co_await chl.send(receiverSize);

  BitVector res1(senderSize);
  BitVector res0(senderSize);
  BitVector tmp1;

  Matrix<u64> curItem(senderSize * receiverSize, dim);
  for (u64 i = 0; i < receiverSize; i++) {
    for (u64 j = 0; j < senderSize; j++) {
      for (u64 k = 0; k < dim; k++) {
        curItem[j * receiverSize + i][k] = data1[i][k];
      }
    }
  }
  if (fake) {
    co_await fakeMatch1(chl, curItem, radius, tmp1);
  } else {
    co_await matchDcmA1(chl, maxLength, height, curItem, tmp1);
  }
  for (u64 i = 0; i < receiverSize; i++) {
    for (u64 j = 0; j < senderSize; j++) {
      res1[i] ^= tmp1[j * receiverSize + i];
    }
  }
  co_await chl.recv(res0);

  u64 matches = 0;
  for (u64 i = 0; i < receiverSize; i++) {
    if (res0[i] ^ res1[i]) {
      matches++;
    }
  }

  cout << "matches: " << matches << endl;
  co_await chl.flush();

  u64 totalComm = chl.bytesSent() + chl.bytesReceived();
  cout << "Total communication: " << totalComm << " bytes, "
      << totalComm / 1024 << " KiB, "
      << totalComm / 1024 / 1024 << " MiB" << endl;
}
