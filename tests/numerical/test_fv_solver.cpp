#include "continuum_lob/finite_volume/fv_solver.hpp"
#include <iostream>
using namespace continuum_lob;
int main() {
  Mesh1D mesh; mesh.n_cells = 32;
  ContinuumState state(mesh);
  for (Index i = 0; i < mesh.n_cells; ++i) {
    Real x = mesh.cell_center(i);
    state.rho[i] = 1.0 + 0.3 * std::exp(-x*x/0.1);
    state.u[i] = 0.05 * x;
  }
  PDEParams pde; SolverConfig scfg; scfg.t_final = 0.05; scfg.dt_max = 0.002;
  FiniteVolumeSolver solver(pde, scfg);
  auto d = solver.run(state, nullptr);
  if (d.steps < 1) { std::cerr << "no steps\n"; return 1; }
  std::cout << "test_fv_solver OK steps=" << d.steps << "\n";
  return 0;
}
