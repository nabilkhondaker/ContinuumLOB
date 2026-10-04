#include "continuum_lob/discrete_lob/order_book.hpp"
#include <iostream>
using namespace continuum_lob;
int main() {
  DiscreteOrderBook book(0.01, 100.0);
  book.add_limit(Side::Bid, 99.95, 100.0, 0.0);
  book.add_limit(Side::Ask, 100.05, 100.0, 0.0);
  auto snap = book.snapshot();
  std::cout << "mid=" << snap.mid << " spread=" << snap.spread << "\n";
  auto fill = book.market_order(Side::Bid, 30.0, 0.0);
  std::cout << "filled=" << fill.filled_qty << " vwap=" << fill.vwap << "\n";
  return 0;
}
