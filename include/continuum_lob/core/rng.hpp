#pragma once
/// \file rng.hpp
/// Explicit, seedable random streams. No uncontrolled global RNG.
/// Author: Nabil Khondaker

#include "continuum_lob/core/types.hpp"
#include <random>
#include <vector>

namespace continuum_lob {

/// Seedable RNG wrapper with independent streams for different subsystems.
class RngStream {
public:
  explicit RngStream(std::uint64_t seed = 0);

  void reseed(std::uint64_t seed);
  std::uint64_t seed() const { return seed_; }

  Real uniform01();
  Real uniform(Real a, Real b);
  Real normal(Real mean = 0.0, Real stddev = 1.0);
  Index discrete_uniform(Index low, Index high_inclusive);
  bool bernoulli(Real p);

  /// Poisson with mean lambda (Knuth for small lambda).
  Index poisson(Real lambda);

private:
  std::uint64_t seed_;
  std::mt19937_64 eng_;
};

/// Manager that owns separate streams for market events, PDE forcing,
/// dataset generation, NN init, RL env, and replay sampling.
class SeedManager {
public:
  explicit SeedManager(std::uint64_t master_seed = 42);

  RngStream& market()   { return market_; }
  RngStream& forcing()  { return forcing_; }
  RngStream& dataset()  { return dataset_; }
  RngStream& nn_init()  { return nn_init_; }
  RngStream& rl_env()   { return rl_env_; }
  RngStream& replay()   { return replay_; }

  std::uint64_t master_seed() const { return master_; }
  void reseed_all(std::uint64_t master);

private:
  std::uint64_t master_;
  RngStream market_, forcing_, dataset_, nn_init_, rl_env_, replay_;
};

}  // namespace continuum_lob
