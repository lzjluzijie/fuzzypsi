#include "fuzzypsi.hpp"

task<> matchDcmA0(cp::Socket& chl, u64 length, u64 height, Matrix<u64>& left,
                  Matrix<u64>& right, BitVector& res0) {
  u64 n = left.rows();
  u64 dim = left.cols();
  u64 nTriples = (dim - 1) * n;
  for (u64 k = 0; k < height; k++) {
    nTriples += min(length - 1, length - k) * 2 * (n * dim);
  }
  Triples triples(nTriples);
  co_await triples.gen0(chl);

  co_await matchDcmA0(chl, length, height, left, right,
                      res0, triples);
}

task<> matchDcmA0(cp::Socket& chl, u64 length, u64 height, Matrix<u64>& left,
                  Matrix<u64>& right, BitVector& res0, Triples& triples) {
  u64 dim = left.cols();
  if (triples.eager) {
    co_await std::move(triples.curTask);
  }

  BitVector tmp0;
  co_await match1dB0(chl, length, height, left, right,
                     tmp0, triples);

  co_await eq0(chl, dim, triples, tmp0, res0);
}

task<> matchDcmA1(cp::Socket& chl, u64 length, u64 height, Matrix<u64>& nums,
                  BitVector& res1) {
  u64 n = nums.rows();
  u64 dim = nums.cols();
  u64 nTriples = (dim - 1) * n;
  for (u64 k = 0; k < height; k++) {
    nTriples += min(length - 1, length - k) * 2 * (n * dim);
  }
  Triples triples(nTriples);
  co_await triples.gen1(chl);

  co_await matchDcmA1(chl, length, height, nums, res1, triples);
}

task<> matchDcmA1(cp::Socket& chl, u64 length, u64 height, Matrix<u64>& nums,
                  BitVector& res1, Triples& triples) {
  u64 dim = nums.cols();
  if (triples.eager) {
    co_await std::move(triples.curTask);
  }

  BitVector tmp1;
  co_await match1dB1(chl, length, height, std::span(nums.data(), nums.size()),
                     tmp1, triples);

  co_await eq1(chl, dim, triples, tmp1, res1);
}