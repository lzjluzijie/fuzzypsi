#include "fuzzypsi.hpp"

Matrix<u64> genDataV0(u64 size, u64 dim, u64 radius, u64 shift) {
  Matrix<u64> data(size, dim);
  for (u64 i = 0; i < size; i++) {
    for (u64 j = 0; j < dim; j++) {
      data[i][j] = i * radius + shift;
    }
  }
  return data;
}

Matrix<u64> genDataV1(u64 size, u64 dim, u64 radius) {
  Matrix<u64> data(size, dim);
  PRNG prng(sysRandomSeed());
  prng.get(std::span(data.data(), data.size()));
  return data;
}

task<> fakeMatch0(cp::Socket& chl, Matrix<u64>& data0, u64 radius,
                  BitVector& res0) {
  co_await chl.send(data0);
  res0.resize(0);
  res0.resize(data0.rows(), 0);
  co_await chl.flush();
}

task<> fakeMatch1(cp::Socket& chl, Matrix<u64>& data1, u64 radius,
                  BitVector& res1) {
  Matrix<u64> data0(data1.rows(), data1.cols());
  co_await chl.recv(data0);

  res1.resize(0);
  res1.resize(data1.rows(), 1);
  for (u64 i = 0; i < data1.rows(); i++) {
    for (u64 j = 0; j < data1.cols(); j++) {
      if (data0[i][j] <= data1[i][j] - radius || data0[i][j] >= data1[i][j] +
          radius) {
        res1[i] = 0;
        break;
      }
    }
  }
  co_await chl.flush();
}

task<> fakeMatchC0(cp::Socket& chl, Matrix<u64>& data0, vector<block>& hashes,
                   BitVector& res0) {
  co_await chl.send(data0);
  co_await chl.send(hashes);
  res0.resize(0);
  res0.resize(data0.rows(), 0);
  co_await chl.recv(res0);
  co_await chl.flush();
}

task<> fakeMatchC1(cp::Socket& chl, Matrix<u64>& data1, vector<block>& hashes,
                   BitVector& res1) {
  Matrix<u64> data0(data1.rows(), data1.cols());
  co_await chl.recv(data0);
  vector<block> hashes0(hashes.size());
  co_await chl.recv(hashes0);

  res1.resize(0);
  res1.resize(data1.rows(), 1);
  for (u64 i = 0; i < data1.rows(); i++) {
    if (hashes0[i] != hashes[i]) {
      res1[i] = 0;
      continue;
    }
    for (u64 j = 0; j < data1.cols(); j++) {
      if ((int64_t)data0[i][j] < 0) {
        if (data1[i][j] > -data0[i][j]) {
          res1[i] = 0;
          break;
        }
      } else {
        if (data1[i][j] < data0[i][j]) {
          u64* ptr = (u64*)&hashes[i];
          res1[i] = 0;
          break;
        }
      }
    }
  }
  co_await chl.send(res1);
  co_await chl.flush();
}


BitVector toBitVector(std::span<u64> data, u64 length) {
  BitVector bv(data.size() * length);
  for (u64 i = 0; i < data.size(); i++) {
    for (u64 j = 0; j < length; j++) {
      bv[i * length + j] = (data[i] >> j) & 1;
    }
  }
  return bv;
}

block toBlock(string str) {
  return block(stoull(str.substr(16, 16), nullptr, 16)
               , stoull(str.substr(0, 16), nullptr, 16));
}
