#include "continuum_lob/coarse_graining/kernel.hpp"
#include <cmath>

namespace continuum_lob {

CoarseGrainKernel::CoarseGrainKernel(KernelType type, Real bandwidth)
    : type_(type), h_(std::max(bandwidth, 1e-12)) {}

Real CoarseGrainKernel::weight(Real dx) const {
  Real r = std::abs(dx) / h_;
  switch (type_) {
    case KernelType::Gaussian:
      return std::exp(-0.5 * r * r) / (h_ * std::sqrt(2.0 * kPi));
    case KernelType::CompactSupport: {
      // Wendland-like C2: (1-r)_+^4 * (4r+1)
      if (r >= 1.0) return 0.0;
      Real t = 1.0 - r;
      return (t * t * t * t) * (4.0 * r + 1.0) / h_;
    }
    case KernelType::Triangular:
      if (r >= 1.0) return 0.0;
      return (1.0 - r) / h_;
    case KernelType::Histogram:
      return (std::abs(dx) < 0.5 * h_) ? (1.0 / h_) : 0.0;
  }
  return 0.0;
}

Real CoarseGrainKernel::support() const {
  switch (type_) {
    case KernelType::Gaussian: return 4.0 * h_;
    case KernelType::CompactSupport:
    case KernelType::Triangular: return h_;
    case KernelType::Histogram: return 0.5 * h_;
  }
  return h_;
}

}  // namespace continuum_lob
