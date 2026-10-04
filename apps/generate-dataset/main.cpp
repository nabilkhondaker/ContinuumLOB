#include "continuum_lob/fields/state.hpp"
#include "continuum_lob/finite_volume/fv_solver.hpp"
#include "continuum_lob/core/rng.hpp"
#include <iostream>
#include <fstream>
using namespace continuum_lob;
int main() {
  std::cout << "Generating tiny synthetic continuum trajectories...\n";
  RngStream rng(42);
  Mesh1D mesh; mesh.n_cells = 32;
  PDEParams pde;
  SolverConfig scfg; scfg.t_final = 0.1; scfg.dt_max = 0.005;
  FiniteVolumeSolver solver(pde, scfg);
  for (int traj = 0; traj < 5; ++traj) {
    ContinuumState state(mesh);
    for (Index i = 0; i < mesh.n_cells; ++i) {
      state.rho[i] = 1.0 + 0.2 * rng.normal();
      state.rho[i] = std::max(0.1, state.rho[i]);
      state.u[i] = 0.05 * rng.normal();
    }
    auto diag = solver.run(state, nullptr);
    std::cout << "traj " << traj << " steps=" << diag.steps
              << " mass_err=" << diag.mass_error << "\n";
  }
  std::cout << "Done (in-memory only; full HDF5/CSV export not required for fixture).\n";
  return 0;
}
