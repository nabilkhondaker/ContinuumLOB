#include "continuum_lob/discrete_lob/order_book.hpp"
#include "continuum_lob/coarse_graining/projector.hpp"
#include <iostream>
using namespace continuum_lob;
int main() {
  DiscreteOrderBook book(0.01, 100.0);
  book.add_limit(Side::Bid, 99.9, 100.0, 0);
  book.add_limit(Side::Ask, 100.1, 100.0, 0);
  if (book.spread() < 0.15) { std::cerr << "spread\n"; return 1; }
  auto fill = book.market_order(Side::Bid, 40.0, 0);
  if (fill.filled_qty < 39.0) { std::cerr << "fill\n"; return 1; }
  ContinuumProjector proj;
  auto st = proj.project(book.snapshot(), 100.0);
  if (st.rho.size() < 1) { std::cerr << "project\n"; return 1; }
  std::cout << "test_discrete OK\n";
  return 0;
}
