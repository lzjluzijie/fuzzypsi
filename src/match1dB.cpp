#include "fuzzypsi.hpp"

task<> match1dB0(cp::Socket& chl, u64 length, u64 height, std::span<u64> left,
                 std::span<u64> right, BitVector& res0, Triples& triples) {
  u64 n = left.size();
  res0.resize(0);
  res0.resize(n, 0);

  Matrix<array<u64, 2> > data(height, n);
  for (u64 i = 0; i < n; i++) {
    u64 curLeft = left[i];
    u64 curRight = right[i];
    for (u64 k = 0; k < height; k++) {
      u64 rightMostChild = ((curLeft + 1) << k) - 1;
      if (rightMostChild > right[i]) {
        data[k][i][0] = -2;
        curLeft = curLeft >> 1;
      } else if (curLeft % 2 == 0) {
        data[k][i][0] = -2;
        curLeft = curLeft >> 1;
      } else {
        data[k][i][0] = curLeft;
        curLeft = (curLeft >> 1) + 1;
      }
      u64 leftMostChild = curRight << k;
      if (leftMostChild < left[i]) {
        data[k][i][1] = -2;
        curRight = curRight >> 1;
      } else if (curRight % 2 == 0) {
        data[k][i][1] = curRight;
        curRight = (curRight >> 1) - 1;
      } else {
        data[k][i][1] = -2;
        curRight = curRight >> 1;
      }
    }
  }

  for (u64 k = 0; k < height; k++) {
    u64 curLength = min(length, length - k + 1);
    BitVector bv = toBitVector(std::span((u64*)data[k].data(), n * 2),
                               curLength);
    BitVector eqRes0;
    co_await eq0(chl, curLength, triples, bv, eqRes0);
    for (u64 i = 0; i < n; i++) {
      res0[i] = res0[i] ^ eqRes0[i * 2] ^ eqRes0[i * 2 + 1];
    }
  }
}

task<> match1dB1(cp::Socket& chl, u64 length, u64 height, std::span<u64> nums,
                 BitVector& res1, Triples& triples) {
  u64 n = nums.size();
  res1.resize(0);
  res1.resize(n, 0);

  Matrix<array<u64, 2> > data(height, n);
  for (u64 i = 0; i < n; i++) {
    u64 curNum = nums[i];
    for (u64 k = 0; k < height; k++) {
      data[k][i][0] = curNum;
      data[k][i][1] = curNum;
      curNum = curNum >> 1;
    }
  }

  for (u64 k = 0; k < height; k++) {
    u64 curLength = min(length, length - k + 1);
    BitVector bv = toBitVector(std::span((u64*)data[k].data(), n * 2),
                               curLength);
    BitVector eqRes1;
    u64 blockSize = bv.sizeBlocks();
    for (u64 i = 0; i < blockSize; i++) {
      bv.blocks()[i] = ~bv.blocks()[i];
    }
    co_await eq1(chl, curLength, triples, bv, eqRes1);
    for (u64 i = 0; i < n; i++) {
      res1[i] = res1[i] ^ eqRes1[i * 2] ^ eqRes1[i * 2 + 1];
    }
  }
}
