#include "continuum_lob/discrete_lob/order_book.hpp"
#include "continuum_lob/coarse_graining/projector.hpp"
#include "continuum_lob/finite_volume/fv_solver.hpp"
#include "continuum_lob/fno/fno_model.hpp"
#include "continuum_lob/execution/execution_env.hpp"
#include "continuum_lob/core/rng.hpp"
#include <iostream>
using namespace continuum_lob;
int main() {
  // discrete -> continuum -> FV step -> FNO forward -> execution step
  DiscreteOrderBook book(0.01, 100.0);
  for (int i = 1; i <= 5; ++i) {
    book.add_limit(Side::Bid, 100.0 - 0.01*i, 20.0*i, 0);
    book.add_limit(Side::Ask, 100.0 + 0.01*i, 20.0*i, 0);
  }
  ContinuumProjector proj;
  auto state = proj.project(book.snapshot(), 100.0);

  PDEParams pde; SolverConfig scfg; scfg.t_final = 0.02; scfg.dt_max = 0.005;
  FiniteVolumeSolver solver(pde, scfg);
  solver.run(state, nullptr);

  RngStream rng(1);
  FNOConfig fcfg; fcfg.n_modes = 4; fcfg.width = 8; fcfg.n_layers = 1;
  FNOModel fno(fcfg, rng);
  Field1D rp(state.mesh), up(state.mesh);
  fno.predict(state.rho, state.u, rp, up);

  ExecutionConfig ecfg; ecfg.n_steps = 3; ecfg.target_qty = 10.0;
  ExecutionEnvironment env(ecfg, pde, scfg);
  env.reset(rng, &state);
  auto a = ExecutionEnvironment::twap_action(env.state(), ecfg);
  env.step(a, rng);

  std::cout << "test_end_to_end OK\n";
  return 0;
}
