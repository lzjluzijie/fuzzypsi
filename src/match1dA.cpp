#include "fuzzypsi.hpp"

task<> match1dA0(cp::Socket& chl, u64 length, std::span<u64> left,
                 std::span<u64> right,
                 BitVector& res0) {
  if (length > 16) {
    cerr << "length should be less than 16" << endl;
    exit(1);
  }
  u64 n = left.size();
  BitVector cmp(length * length * 2 * n);
  array<u16, 2>* ptr = (array<u16, 2>*)cmp.data();
  for (u64 i = 0; i < n; i++) {
    u64 curLeft = left[i];
    u64 curRight = right[i];
    for (u64 k = 0; k < length; k++) {
      u64 rightMostChild = ((curLeft + 1) << k) - 1;
      if (rightMostChild > right[i]) {
        ptr[i * length + k][0] = -2;
        curLeft = curLeft >> 1;
      } else if (curLeft % 2 == 0) {
        ptr[i * length + k][0] = -2;
        curLeft = curLeft >> 1;
      } else {
        ptr[i * length + k][0] = curLeft;
        curLeft = (curLeft >> 1) + 1;
      }
      u64 leftMostChild = curRight << k;
      if (leftMostChild < left[i]) {
        ptr[i * length + k][1] = -2;
        curRight = curRight >> 1;
      } else if (curRight % 2 == 0) {
        ptr[i * length + k][1] = curRight;
        curRight = (curRight >> 1) - 1;
      } else {
        ptr[i * length + k][1] = -2;
        curRight = curRight >> 1;
      }
    }
  }

  // u64 item = 0;
  // for (u64 k = 0; k < height; k++) {
  //   cout << "level " << k << " " << ptr[item * height + k][0] << " "
  //       << ptr[item * height + k][1] << endl;
  // }

  u64 nTriples = (length - 1) * length * 2 * n;
  Triples triples(nTriples);
  BitVector eqRes0;
  co_await triples.gen0(chl);
  co_await eq0(chl, length, triples, cmp, eqRes0);

  res0.resize(0);
  res0.resize(n, 0);
  for (u64 i = 0; i < n; i++) {
    for (u64 j = 0; j < length * 2; j++) {
      res0[i] = res0[i] ^ eqRes0[i * length * 2 + j];
    }
  }
}

task<> match1dA1(cp::Socket& chl, u64 length, std::span<u64> nums,
                 BitVector& res1) {
  if (length > 16) {
    cerr << "length should be less than 16" << endl;
    exit(1);
  }
  u64 n = nums.size();
  BitVector cmp(length * length * 2 * n);
  array<u16, 2>* ptr = (array<u16, 2>*)cmp.data();
  for (u64 i = 0; i < n; i++) {
    u64 curNum = nums[i];
    for (u64 k = 0; k < length; k++) {
      ptr[i * length + k][0] = curNum;
      ptr[i * length + k][1] = curNum;
      curNum = curNum >> 1;
    }
  }

  // u64 item = 0;
  // for (u64 k = 0; k < height; k++) {
  //   cout << "level " << k << " " << ptr[item * height + k][0] << " "
  //       << ptr[item * height + k][1] << endl;
  // }

  u64 nTriples = (length - 1) * length * 2 * n;
  Triples triples(nTriples);
  BitVector eqRes1;
  co_await triples.gen1(chl);
  u64 blockSize = cmp.sizeBlocks();
  for (u64 i = 0; i < blockSize; i++) {
    cmp.blocks()[i] = ~cmp.blocks()[i];
  }
  co_await eq1(chl, length, triples, cmp, eqRes1);

  res1.resize(0);
  res1.resize(n, 0);
  for (u64 i = 0; i < n; i++) {
    for (u64 j = 0; j < length * 2; j++) {
      res1[i] = res1[i] ^ eqRes1[i * length * 2 + j];
    }
  }
}
