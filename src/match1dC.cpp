#include "fuzzypsi.hpp"

task<> match1dC0(cp::Socket& chl, u64 length, std::span<u64> nums,
                 BitVector& res0, Triples& triples) {
  u64 n = nums.size();
  res0.resize(0);
  res0.resize(n, 0);

  Matrix<u64> data(length, n);
  for (u64 i = 0; i < n; i++) {
    if ((int64_t)nums[i] > 0) {
      u64 curLeft = nums[i];
      for (u64 k = 0; k < length; k++) {
        if (curLeft % 2 == 0) {
          data[k][i] = -2;
          curLeft = curLeft >> 1;
        } else {
          data[k][i] = curLeft;
          curLeft = (curLeft >> 1) + 1;
        }
      }
    } else if (nums[i] == 0) {
      for (u64 k = 0; k < length - 1; k++) {
        data[k][i] = -2;
      }
      data[length - 1][i] = 0;
    } else {
      u64 curRight = -nums[i];
      for (u64 k = 0; k < length; k++) {
        if (curRight % 2 == 0) {
          data[k][i] = curRight;
          curRight = (curRight >> 1) - 1;
        } else {
          data[k][i] = -2;
          curRight = curRight >> 1;
        }
      }
    }
  }

  for (u64 k = 0; k < length; k++) {
    u64 curLength = min(length, length - k + 1);
    BitVector bv = toBitVector(std::span((u64*)data[k].data(), n),
                               curLength);
    BitVector eqRes0;
    co_await eq0(chl, curLength, triples, bv, eqRes0);
    for (u64 i = 0; i < n; i++) {
      res0[i] = res0[i] ^ eqRes0[i];
    }
  }
}

task<> match1dC1(cp::Socket& chl, u64 length, std::span<u64> nums,
                 BitVector& res1, Triples& triples) {
  u64 n = nums.size();
  res1.resize(0);
  res1.resize(n, 0);

  Matrix<u64> data(length, n);
  for (u64 i = 0; i < n; i++) {
    u64 curNum = nums[i];
    for (u64 k = 0; k < length; k++) {
      data[k][i] = curNum;
      curNum = curNum >> 1;
    }
  }

  for (u64 k = 0; k < length; k++) {
    u64 curLength = min(length, length - k + 1);
    BitVector bv = toBitVector(std::span((u64*)data[k].data(), n),
                               curLength);
    BitVector eqRes1;
    u64 blockSize = bv.sizeBlocks();
    for (u64 i = 0; i < blockSize; i++) {
      bv.blocks()[i] = ~bv.blocks()[i];
    }
    co_await eq1(chl, curLength, triples, bv, eqRes1);
    for (u64 i = 0; i < n; i++) {
      res1[i] = res1[i] ^ eqRes1[i];
    }
  }
}
