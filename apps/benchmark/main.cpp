#include "continuum_lob/finite_volume/fv_solver.hpp"
#include "continuum_lob/fno/fno_model.hpp"
#include "continuum_lob/core/rng.hpp"
#include <iostream>
#include <chrono>
using namespace continuum_lob;
int main() {
  Mesh1D mesh; mesh.n_cells = 128;
  ContinuumState state(mesh);
  for (Index i = 0; i < mesh.n_cells; ++i) state.rho[i] = 1.0;
  PDEParams pde; SolverConfig scfg; scfg.t_final = 0.05; scfg.dt_max = 0.001;
  FiniteVolumeSolver solver(pde, scfg);
  auto t0 = std::chrono::steady_clock::now();
  solver.run(state, nullptr);
  auto t1 = std::chrono::steady_clock::now();
  double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
  std::cout << "FV solver 128 cells, t=0.05: " << ms << " ms\n";

  RngStream rng(0);
  FNOConfig cfg; cfg.n_modes = 16; cfg.width = 32; cfg.n_layers = 4;
  FNOModel model(cfg, rng);
  Field1D rho(mesh, 1.0), u(mesh, 0.0), rp(mesh), up(mesh);
  t0 = std::chrono::steady_clock::now();
  for (int i = 0; i < 20; ++i) model.predict(rho, u, rp, up);
  t1 = std::chrono::steady_clock::now();
  ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
  std::cout << "FNO 20 forwards (untrained): " << ms << " ms\n";
  return 0;
}
