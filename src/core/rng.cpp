#include "continuum_lob/core/rng.hpp"
#include <cmath>

namespace continuum_lob {

RngStream::RngStream(std::uint64_t seed) : seed_(seed), eng_(seed) {}

void RngStream::reseed(std::uint64_t seed) {
  seed_ = seed;
  eng_.seed(seed);
}

Real RngStream::uniform01() {
  std::uniform_real_distribution<Real> dist(0.0, 1.0);
  return dist(eng_);
}

Real RngStream::uniform(Real a, Real b) {
  std::uniform_real_distribution<Real> dist(a, b);
  return dist(eng_);
}

Real RngStream::normal(Real mean, Real stddev) {
  std::normal_distribution<Real> dist(mean, stddev);
  return dist(eng_);
}

Index RngStream::discrete_uniform(Index low, Index high_inclusive) {
  std::uniform_int_distribution<Index> dist(low, high_inclusive);
  return dist(eng_);
}

bool RngStream::bernoulli(Real p) {
  return uniform01() < p;
}

Index RngStream::poisson(Real lambda) {
  if (lambda <= 0.0) return 0;
  // Knuth algorithm for moderate lambda
  if (lambda < 30.0) {
    Real L = std::exp(-lambda);
    Index k = 0;
    Real p = 1.0;
    do {
      ++k;
      p *= uniform01();
    } while (p > L);
    return k - 1;
  }
  // Normal approximation for large lambda
  Real n = normal(lambda, std::sqrt(lambda));
  return static_cast<Index>(std::max(0.0, std::round(n)));
}

SeedManager::SeedManager(std::uint64_t master_seed)
    : master_(master_seed),
      market_(master_seed + 1),
      forcing_(master_seed + 2),
      dataset_(master_seed + 3),
      nn_init_(master_seed + 4),
      rl_env_(master_seed + 5),
      replay_(master_seed + 6) {}

void SeedManager::reseed_all(std::uint64_t master) {
  master_ = master;
  market_.reseed(master + 1);
  forcing_.reseed(master + 2);
  dataset_.reseed(master + 3);
  nn_init_.reseed(master + 4);
  rl_env_.reseed(master + 5);
  replay_.reseed(master + 6);
}

}  // namespace continuum_lob
