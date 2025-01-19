#include "fuzzypsi.hpp"

task<> matchDcmC0(cp::Socket& chl, u64 length, Matrix<u64>& nums,
                  vector<block>& hashes, BitVector& res0) {
  u64 n = nums.rows();
  u64 dim = nums.cols();
  u64 nTriples = (dim - 1 + hashLength) * n;
  for (u64 k = 0; k < length; k++) {
    nTriples += min(length - 1, length - k) * nums.size();
  }
  Triples* triples = new Triples(nTriples);
  triples->eagerGen0(chl);

  co_await matchDcmC0(chl, length, nums, hashes, res0,
                      *triples);
  delete triples;
}

task<> matchDcmC0(cp::Socket& chl, u64 length, Matrix<u64>& nums,
                  vector<block>& hashes, BitVector& res0, Triples& triples) {
  u64 n = nums.rows();
  u64 dim = nums.cols();
  if (triples.eager) {
    co_await std::move(triples.curTask);
  }

  BitVector tmp0;
  co_await match1dC0(chl, length, std::span(nums.data(), nums.size()), tmp0,
                     triples);

  BitVector in0(n * (dim + hashLength));
  for (u64 i = 0; i < n; i++) {
    for (u64 j = 0; j < dim; j++) {
      in0[i * (dim + hashLength) + j] = tmp0[i * dim + j];
    }
    for (u64 j = 0; j < hashLength; j++) {
      in0[i * (dim + hashLength) + dim + j] =
          (hashes[i] & (OneBlock << j)) != ZeroBlock;
    }
  }

  co_await eq0(chl, dim + hashLength, triples, in0, res0);
}

task<> matchDcmC1(cp::Socket& chl, u64 length, Matrix<u64>& nums,
                  vector<block>& hashes, BitVector& res1) {
  u64 n = nums.rows();
  u64 dim = nums.cols();
  u64 nTriples = (dim - 1 + hashLength) * n;
  for (u64 k = 0; k < length; k++) {
    nTriples += min(length - 1, length - k) * nums.size();
  }
  Triples* triples = new Triples(nTriples);
  triples->eagerGen1(chl);

  co_await matchDcmC1(chl, length, nums, hashes, res1,
                      *triples);
  delete triples;
}

task<> matchDcmC1(cp::Socket& chl, u64 length, Matrix<u64>& nums,
                  vector<block>& hashes, BitVector& res1, Triples& triples) {
  u64 n = nums.rows();
  u64 dim = nums.cols();
  if (triples.eager) {
    co_await std::move(triples.curTask);
  }

  BitVector tmp1;
  co_await match1dC1(chl, length, std::span(nums.data(), nums.size()), tmp1,
                     triples);

  BitVector in1(n * (dim + hashLength));
  for (u64 i = 0; i < n; i++) {
    for (u64 j = 0; j < dim; j++) {
      in1[i * (dim + hashLength) + j] = tmp1[i * dim + j];
    }
    for (u64 j = 0; j < hashLength; j++) {
      in1[i * (dim + hashLength) + dim + j] =
          (hashes[i] & (OneBlock << j)) == ZeroBlock;
    }
  }

  co_await eq1(chl, dim + hashLength, triples, in1, res1);
}
