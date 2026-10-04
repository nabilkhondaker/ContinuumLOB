#include "continuum_lob/execution/execution_env.hpp"
#include "continuum_lob/core/rng.hpp"
#include <iostream>
using namespace continuum_lob;
int main() {
  RngStream rng(99);
  ExecutionConfig cfg; cfg.n_steps = 5; cfg.target_qty = 20.0;
  ExecutionEnvironment env(cfg);
  env.reset(rng);
  Real R = 0;
  for (int i = 0; i < cfg.n_steps; ++i) {
    auto a = ExecutionEnvironment::twap_action(env.state(), cfg);
    R += env.step(a, rng).reward;
  }
  std::cout << "execution episode reward=" << R << " remaining=" << env.state().remaining << "\n";
  return 0;
}
