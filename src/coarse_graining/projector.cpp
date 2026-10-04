#include "continuum_lob/coarse_graining/projector.hpp"
#include <cmath>

namespace continuum_lob {

ContinuumProjector::ContinuumProjector(ProjectorConfig cfg)
    : cfg_(std::move(cfg)),
      kernel_(cfg_.kernel, cfg_.bandwidth) {}

ContinuumState ContinuumProjector::project(const BookSnapshot& snap, Real mid_price) const {
  Mesh1D mesh;
  mesh.x_min = -cfg_.domain_half_width;
  mesh.x_max =  cfg_.domain_half_width;
  mesh.n_cells = cfg_.n_cells;

  ContinuumState state(mesh);
  if (mid_price < 1e-12) mid_price = 1.0;

  auto add_orders = [&](const std::vector<BookLevel>& levels, Field1D& rho_side, bool is_bid) {
    for (const auto& lvl : levels) {
      Real x = (lvl.price - mid_price) / mid_price;
      // Optionally flip sign convention; keep geometric x
      for (Index i = 0; i < mesh.n_cells; ++i) {
        Real xc = mesh.cell_center(i);
        Real w = kernel_.weight(xc - x);
        rho_side[i] += lvl.total_qty * w;
      }
      (void)is_bid;
    }
  };

  add_orders(snap.bids, state.rho_bid, true);
  add_orders(snap.asks, state.rho_ask, false);

  // Unified density: total liquidity
  for (Index i = 0; i < mesh.n_cells; ++i) {
    state.rho[i] = state.rho_bid[i] + state.rho_ask[i];
  }

  // Velocity left at zero for static snapshot; dynamic flow estimated from event rates elsewhere
  state.u.zero();
  state.u_bid.zero();
  state.u_ask.zero();
  state.time = snap.time;
  return state;
}

}  // namespace continuum_lob
