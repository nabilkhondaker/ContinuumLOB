#pragma once
/// \file execution_env.hpp
/// Optimal execution environment using continuum state as observation.

#include "continuum_lob/fields/state.hpp"
#include "continuum_lob/finite_volume/fv_solver.hpp"
#include "continuum_lob/core/rng.hpp"
#include <vector>

namespace continuum_lob {

struct ExecutionConfig {
  Real target_qty{100.0};       // shares to execute (sell > 0)
  Real horizon{1.0};            // time horizon
  Real arrival_price{100.0};
  Real inv_penalty{0.01};
  Real impact_coeff{0.001};
  Real adverse_coeff{0.001};
  Real latency{0.001};
  Index n_steps{50};
  bool use_surrogate{false};
};

struct ExecutionState {
  ContinuumState continuum;
  Real inventory{0.0};
  Real cash{0.0};
  Real remaining{0.0};
  Real time{0.0};
  Real mid{100.0};
  Real spread{0.02};
  bool done{false};
};

struct ExecutionAction {
  Real price_offset{0.0};   // relative to mid
  Real quantity{0.0};       // size to place
  Real market_fraction{0.0}; // fraction as market order
};

struct ExecutionStepResult {
  Real reward{0.0};
  Real shortfall{0.0};
  Real impact{0.0};
  Real adverse{0.0};
  Real fill_qty{0.0};
  bool done{false};
};

class ExecutionEnvironment {
public:
  explicit ExecutionEnvironment(ExecutionConfig cfg, PDEParams pde = {},
                                SolverConfig solver = {});

  void reset(RngStream& rng, ContinuumState* initial = nullptr);
  ExecutionStepResult step(const ExecutionAction& action, RngStream& rng);

  const ExecutionState& state() const { return state_; }
  const ExecutionConfig& config() const { return cfg_; }

  /// Baseline: TWAP-style equal slices.
  static ExecutionAction twap_action(const ExecutionState& s, const ExecutionConfig& cfg);
  /// Immediate market execution of remaining.
  static ExecutionAction market_all(const ExecutionState& s);

private:
  ExecutionConfig cfg_;
  PDEParams pde_params_;
  SolverConfig solver_cfg_;
  FiniteVolumeSolver solver_;
  ExecutionState state_;
  Real arrival_{0.0};
};

}  // namespace continuum_lob
