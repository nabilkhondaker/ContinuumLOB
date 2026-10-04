#include "continuum_lob/fields/field1d.hpp"
#include <iostream>
#include <cmath>
using namespace continuum_lob;

int main() {
  Mesh1D m; m.n_cells = 10; m.x_min = 0; m.x_max = 1;
  Field1D f(m, 2.0);
  if (std::abs(f.sum() - 20.0) > 1e-12) { std::cerr << "sum fail\n"; return 1; }
  if (std::abs(f.integral() - 2.0) > 1e-12) { std::cerr << "integral fail\n"; return 1; }
  f *= 0.5;
  if (std::abs(f.max() - 1.0) > 1e-12) { std::cerr << "max fail\n"; return 1; }
  std::cout << "test_fields OK\n";
  return 0;
}
