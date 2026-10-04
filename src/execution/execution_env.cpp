#include "continuum_lob/execution/execution_env.hpp"
#include <algorithm>
#include <cmath>

namespace continuum_lob {

ExecutionEnvironment::ExecutionEnvironment(ExecutionConfig cfg, PDEParams pde,
                                           SolverConfig solver)
    : cfg_(std::move(cfg)),
      pde_params_(std::move(pde)),
      solver_cfg_(std::move(solver)),
      solver_(pde_params_, solver_cfg_) {}

void ExecutionEnvironment::reset(RngStream& rng, ContinuumState* initial) {
  state_ = ExecutionState{};
  if (initial) {
    state_.continuum = *initial;
  } else {
    Mesh1D mesh;
    mesh.x_min = -1.0;
    mesh.x_max = 1.0;
    mesh.n_cells = 32;
    state_.continuum = ContinuumState(mesh);
    for (Index i = 0; i < mesh.n_cells; ++i) {
      Real x = mesh.cell_center(i);
      state_.continuum.rho[i] = 1.0 + 0.3 * std::exp(-x * x / 0.1) + 0.05 * rng.normal();
      state_.continuum.rho[i] = std::max(state_.continuum.rho[i], 0.1);
      state_.continuum.u[i] = 0.02 * rng.normal();
    }
  }
  state_.inventory = 0.0;
  state_.cash = 0.0;
  state_.remaining = cfg_.target_qty;
  state_.time = 0.0;
  state_.mid = cfg_.arrival_price;
  state_.spread = 0.02;
  state_.done = false;
  arrival_ = cfg_.arrival_price;
  solver_cfg_.t_final = cfg_.horizon / static_cast<Real>(cfg_.n_steps);
}

ExecutionAction ExecutionEnvironment::twap_action(const ExecutionState& s,
                                                  const ExecutionConfig& cfg) {
  ExecutionAction a;
  Real slice = cfg.target_qty / static_cast<Real>(cfg.n_steps);
  a.quantity = std::min(slice, s.remaining);
  a.price_offset = -0.5 * s.spread;
  a.market_fraction = 0.0;
  return a;
}

ExecutionAction ExecutionEnvironment::market_all(const ExecutionState& s) {
  ExecutionAction a;
  a.quantity = s.remaining;
  a.market_fraction = 1.0;
  a.price_offset = 0.0;
  return a;
}

ExecutionStepResult ExecutionEnvironment::step(const ExecutionAction& action, RngStream& rng) {
  ExecutionStepResult res;
  if (state_.done || state_.remaining <= 1e-8) {
    res.done = true;
    state_.done = true;
    return res;
  }

  Real qty = std::max(0.0, std::min(action.quantity, state_.remaining));
  Real mkt_frac = std::clamp(action.market_fraction, 0.0, 1.0);
  Real mkt_qty = qty * mkt_frac;
  Real lim_qty = qty - mkt_qty;

  Real local_liq = 0.0;
  const auto& rho = state_.continuum.rho;
  Index n = rho.size();
  for (Index i = 0; i < n; ++i) local_liq += rho[i];
  local_liq = std::max(local_liq / static_cast<Real>(n), 0.1);

  Real impact = cfg_.impact_coeff * mkt_qty / local_liq;
  Real fill_price_mkt = state_.mid - impact;
  Real filled = mkt_qty;

  Real fill_prob = std::clamp(0.3 + 0.5 * (local_liq / 2.0) - 2.0 * std::abs(action.price_offset), 0.0, 1.0);
  if (rng.uniform01() < fill_prob) {
    filled += lim_qty;
  }
  filled = std::min(filled, state_.remaining);

  Real avg_px = (filled > 1e-12) ? fill_price_mkt : state_.mid;
  Real adverse = cfg_.adverse_coeff * rng.normal() * filled;

  state_.inventory -= filled;
  state_.cash += filled * avg_px;
  state_.remaining -= filled;
  state_.mid += rng.normal(0.0, 0.01) - 0.1 * impact;
  state_.time += cfg_.horizon / static_cast<Real>(cfg_.n_steps);

  SourceTerms src;
  src.S_rho = Field1D(state_.continuum.mesh, 0.0);
  src.S_u = Field1D(state_.continuum.mesh, 0.0);
  src.F_ext = Field1D(state_.continuum.mesh, 0.0);
  Index mid_cell = n / 2;
  if (mid_cell < n) {
    src.S_rho[mid_cell] = -0.01 * filled;
  }
  Real dt = solver_cfg_.t_final;
  try {
    solver_.step(state_.continuum, src, dt);
  } catch (...) {
  }

  Real shortfall = (arrival_ - avg_px) * filled;
  Real inv_pen = cfg_.inv_penalty * state_.remaining * state_.remaining;
  res.reward = -std::abs(shortfall) - inv_pen - std::abs(impact) * filled - std::abs(adverse);
  res.shortfall = shortfall;
  res.impact = impact;
  res.adverse = adverse;
  res.fill_qty = filled;
  res.done = (state_.remaining <= 1e-8) || (state_.time >= cfg_.horizon - 1e-12);
  state_.done = res.done;
  return res;
}

}  // namespace continuum_lob
