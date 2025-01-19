#pragma once

#include <coproto/Socket/AsioSocket.h>
#include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Common/Matrix.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Crypto/PRNG.h>
#include <cryptoTools/Crypto/RandomOracle.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/IOService.h>
#include <libOTe/NChooseOne/Kkrt/KkrtNcoOtReceiver.h>
#include <libOTe/NChooseOne/Kkrt/KkrtNcoOtSender.h>
#include <libOTe/TwoChooseOne/Iknp/IknpOtExtReceiver.h>
#include <libOTe/TwoChooseOne/Iknp/IknpOtExtSender.h>
#include <libOTe/TwoChooseOne/Silent/SilentOtExtReceiver.h>
#include <libOTe/TwoChooseOne/Silent/SilentOtExtSender.h>
#include <openssl/sha.h>

#include <iostream>

#include "cryptoTools/Common/CuckooIndex.h"

using namespace std;
using namespace oc;
using u128 = unsigned __int128;

inline u64 hashLength = 128;

inline bool b(block x) { return x.data()[0] & 1; }

struct alignas(16) HASH {
  unsigned char data[SHA256_DIGEST_LENGTH];
};

struct alignas(16) HASH64 {
  unsigned char data[SHA512_DIGEST_LENGTH];
};

Matrix<u64> genDataV0(u64 size, u64 dim, u64 radius, u64 shift = 1000);

Matrix<u64> genDataV1(u64 size, u64 dim, u64 radius);

task<> fakeMatch0(cp::Socket& chl, Matrix<u64>& data0, u64 radius,
                  BitVector& res0);

task<> fakeMatch1(cp::Socket& chl, Matrix<u64>& data1, u64 radius,
                  BitVector& res1);

task<> fakeMatchC0(cp::Socket& chl, Matrix<u64>& data0, vector<block>& hashes,
                   BitVector& res0);

task<> fakeMatchC1(cp::Socket& chl, Matrix<u64>& data1, vector<block>& hashes,
                   BitVector& res1);

BitVector toBitVector(std::span<u64> data, u64 length);

block toBlock(string str);

vector<u64> simulate_probability(u64 n, u64 m, u64 hash_functions = 3,
                                 u64 trials = 10000);

task<> triple0(cp::Socket& chl, BitVector& a0, BitVector& b0, BitVector& c0);

task<> triple1(cp::Socket& chl, BitVector& a1, BitVector& b1, BitVector& c1);

struct Triples {
  u64 nTriples;
  BitVector a, b, c;
  bool fake = false;

  u64 curTriple = 0;

  bool eager = false;
  macoro::eager_task<> curTask;

  Triples(u64 nTriples)
    : nTriples(nTriples), a(nTriples), b(nTriples), c(nTriples) {
  }

  bool curA() { return a[curTriple]; }

  bool curB() { return b[curTriple]; }

  bool curC() { return c[curTriple]; }

  void move(i64 offset) { curTriple += offset; }

  task<> gen0(cp::Socket& chl) {
    if (!fake) {
      co_await triple0(chl, a, b, c);
    }
  }

  task<> gen1(cp::Socket& chl) {
    if (!fake) {
      co_await triple1(chl, a, b, c);
    }
  }

  void eagerGen0(cp::Socket& chl) {
    eager = true;
    curTask = std::move(gen0(chl) | macoro::make_eager());
  }

  void eagerGen1(cp::Socket& chl) {
    eager = true;
    curTask = std::move(gen1(chl) | macoro::make_eager());
  }
};

struct Fuzzy {
  bool fake = false;
  u64 maxLength;
  u64 radius;
  u64 height;
  u64 dim;
  u64 binSize;

  Fuzzy(u64 maxLength, u64 radius, u64 dim, u64 binSize)
    : maxLength(maxLength), radius(radius), height(log2ceil(2 * radius + 1)),
      dim(dim), binSize(binSize) {
  }

  task<> matchA0(cp::Socket& chl, Matrix<u64>& data0);

  task<> matchA1(cp::Socket& chl, Matrix<u64>& data1);

  task<> matchB0(cp::Socket& chl, Matrix<u64>& data0);

  task<> matchB1(cp::Socket& chl, Matrix<u64>& data1);

  task<> matchC0(cp::Socket& chl, Matrix<u64>& data0);

  task<> matchC1(cp::Socket& chl, Matrix<u64>& data1);

  void bench(string protocol, u64& duration, u64 senderSize, u64 receiverSize);
};

task<> eq0(cp::Socket& chl, u64 length, Triples& triples, BitVector& in0,
           BitVector& res0);

task<> eq1(cp::Socket& chl, u64 length, Triples& triples, BitVector& in1,
           BitVector& res1);

task<> match1dA0(cp::Socket& chl, u64 length, std::span<u64> left,
                 std::span<u64> right, BitVector& res0);

task<> match1dA1(cp::Socket& chl, u64 length, std::span<u64> nums,
                 BitVector& res1);

task<> match1dB0(cp::Socket& chl, u64 length, u64 height, std::span<u64> left,
                 std::span<u64> right, BitVector& res0, Triples& triples);

task<> match1dB1(cp::Socket& chl, u64 length, u64 height, std::span<u64> nums,
                 BitVector& res1, Triples& triples);

task<> match1dC0(cp::Socket& chl, u64 length, std::span<u64> nums,
                 BitVector& res0, Triples& triples);

task<> match1dC1(cp::Socket& chl, u64 length, std::span<u64> nums,
                 BitVector& res1, Triples& triples);

task<> matchDcmA0(cp::Socket& chl, u64 length, u64 height, Matrix<u64>& left,
                  Matrix<u64>& right, BitVector& res0);

task<> matchDcmA1(cp::Socket& chl, u64 length, u64 height, Matrix<u64>& nums,
                  BitVector& res1);

task<> matchDcmA0(cp::Socket& chl, u64 length, u64 height, Matrix<u64>& left,
                  Matrix<u64>& right, BitVector& res0, Triples& triples);

task<> matchDcmA1(cp::Socket& chl, u64 length, u64 height, Matrix<u64>& nums,
                  BitVector& res1, Triples& triples);

task<> matchDcmC0(cp::Socket& chl, u64 length, Matrix<u64>& nums,
                  vector<block>& hashes, BitVector& res0);

task<> matchDcmC0(cp::Socket& chl, u64 length, Matrix<u64>& nums,
                  vector<block>& hashes, BitVector& res0, Triples& triples);

task<> matchDcmC1(cp::Socket& chl, u64 length, Matrix<u64>& nums,
                  vector<block>& hashes, BitVector& res1);

task<> matchDcmC1(cp::Socket& chl, u64 length, Matrix<u64>& nums,
                  vector<block>& hashes, BitVector& res1, Triples& triples);

void test();

void bench(string protocol, u64 senderSize, u64 receiverSize, u64 dim,
           u64 radius, u64 binSize);
