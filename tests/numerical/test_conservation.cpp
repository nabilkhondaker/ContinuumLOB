#include "continuum_lob/finite_volume/fv_solver.hpp"
#include <iostream>
#include <cmath>
using namespace continuum_lob;

int main() {
  Mesh1D mesh; mesh.n_cells = 64; mesh.x_min = -1; mesh.x_max = 1;
  ContinuumState state(mesh);
  for (Index i = 0; i < mesh.n_cells; ++i) {
    state.rho[i] = 1.0;
    state.u[i] = 0.0;
  }
  PDEParams pde;
  pde.bc = BCType::ZeroFlux;
  SolverConfig scfg;
  scfg.t_final = 0.1;
  scfg.cfl = 0.3;
  scfg.dt_max = 0.005;
  FiniteVolumeSolver solver(pde, scfg);
  auto diag = solver.run(state, nullptr);
  // With zero velocity and zero sources, mass should be nearly conserved
  if (diag.mass_error > 1e-6) {
    std::cerr << "mass_error too large: " << diag.mass_error << "\n";
    return 1;
  }
  std::cout << "test_conservation OK mass_error=" << diag.mass_error << "\n";
  return 0;
}
