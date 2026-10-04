#include "continuum_lob/fno/fno_model.hpp"
#include "continuum_lob/core/rng.hpp"
#include <iostream>
using namespace continuum_lob;
int main() {
  RngStream rng(0);
  FNOConfig cfg; cfg.n_modes = 4; cfg.width = 8; cfg.n_layers = 1;
  FNOModel m(cfg, rng);
  Mesh1D mesh; mesh.n_cells = 16;
  Field1D rho(mesh, 1.0), u(mesh, 0.0), rp(mesh), up(mesh);
  m.predict(rho, u, rp, up);
  if (rp.size() != 16) { std::cerr << "size\n"; return 1; }
  std::cout << "test_fno OK params=" << m.parameter_count() << "\n";
  return 0;
}
