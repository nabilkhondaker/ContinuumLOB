#include "continuum_lob/fno/fno_model.hpp"
#include "continuum_lob/core/rng.hpp"
#include <iostream>
using namespace continuum_lob;
int main() {
  RngStream rng(1);
  FNOConfig cfg; cfg.n_modes = 4; cfg.width = 8; cfg.n_layers = 1;
  FNOModel model(cfg, rng);
  Mesh1D mesh; mesh.n_cells = 16;
  Field1D rho(mesh, 1.0), u(mesh, 0.0), rp(mesh), up(mesh);
  model.predict(rho, u, rp, up);
  std::cout << "surrogate forward OK, params=" << model.parameter_count() << "\n";
  return 0;
}
