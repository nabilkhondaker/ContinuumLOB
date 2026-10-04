#pragma once
/// \file kernel.hpp
/// Smoothing kernels for discrete-event → continuum field projection.

#include "continuum_lob/core/types.hpp"
#include <cmath>

namespace continuum_lob {

class CoarseGrainKernel {
public:
  explicit CoarseGrainKernel(KernelType type = KernelType::Gaussian, Real bandwidth = 0.05);

  Real bandwidth() const { return h_; }
  KernelType type() const { return type_; }

  /// Kernel weight K(x - x_i) / normalization is handled by caller for density.
  Real weight(Real dx) const;

  /// Support radius beyond which weight is treated as zero.
  Real support() const;

private:
  KernelType type_;
  Real h_;
};

}  // namespace continuum_lob
