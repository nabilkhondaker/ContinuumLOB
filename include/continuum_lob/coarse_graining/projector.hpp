#pragma once
/// \file projector.hpp
/// Projects discrete order-book state onto continuum density/velocity fields.

#include "continuum_lob/coarse_graining/kernel.hpp"
#include "continuum_lob/discrete_lob/order_book.hpp"
#include "continuum_lob/fields/state.hpp"

namespace continuum_lob {

struct ProjectorConfig {
  KernelType kernel{KernelType::Gaussian};
  Real bandwidth{0.05};
  Real domain_half_width{1.0};  // x in [-L, L] in normalized price units
  Index n_cells{64};
};

class ContinuumProjector {
public:
  explicit ContinuumProjector(ProjectorConfig cfg = {});

  /// Map a discrete book snapshot to continuum density (bid/ask separately).
  /// x = (price - mid) / mid   (normalized displacement)
  ContinuumState project(const BookSnapshot& snap, Real mid_price) const;

  const ProjectorConfig& config() const { return cfg_; }

private:
  ProjectorConfig cfg_;
  CoarseGrainKernel kernel_;
};

}  // namespace continuum_lob
