#pragma once
/// \file fno_model.hpp
/// Lightweight Fourier Neural Operator (1-D) implemented in pure C++20.
/// This is a research prototype: weights are dense arrays, forward pass is
/// explicit. Suitable for small grids and reproducibility experiments.
/// Author: Nabil Khondaker

#include "continuum_lob/fields/field1d.hpp"
#include "continuum_lob/core/rng.hpp"
#include <vector>
#include <cmath>
#include <complex>

namespace continuum_lob {

struct FNOConfig {
  Index n_modes{16};       // number of retained Fourier modes
  Index width{32};         // channel width
  Index n_layers{4};       // Fourier layers
  Index n_input_channels{2};  // e.g. rho, u
  Index n_output_channels{2};
  Real learning_rate{1e-3};
};

/// Spectral convolution in 1-D: multiply low modes by complex weights.
class SpectralConv1D {
public:
  SpectralConv1D(Index n_modes, Index in_ch, Index out_ch, RngStream& rng);

  /// in/out: (channels, spatial)
  void forward(const std::vector<std::vector<Real>>& in,
               std::vector<std::vector<Real>>& out) const;

  Index n_modes() const { return n_modes_; }
  Index in_channels() const { return in_ch_; }
  Index out_channels() const { return out_ch_; }

  // Weights: [out_ch][in_ch][n_modes] complex
  std::vector<std::vector<std::vector<std::complex<Real>>>> weights;

private:
  Index n_modes_, in_ch_, out_ch_;
};

class FNOModel {
public:
  explicit FNOModel(FNOConfig cfg, RngStream& rng);

  const FNOConfig& config() const { return cfg_; }

  /// Forward: input fields stacked as channels → output fields.
  /// Input size must match mesh; output written to out_fields.
  void forward(const std::vector<Field1D>& in_fields,
               std::vector<Field1D>& out_fields) const;

  /// Convenience: (rho, u) → (rho_pred, u_pred)
  void predict(const Field1D& rho, const Field1D& u,
               Field1D& rho_pred, Field1D& u_pred) const;

  Index parameter_count() const;

  /// Simple MSE loss between prediction and target (for offline training loops).
  Real mse_loss(const std::vector<Field1D>& pred,
                const std::vector<Field1D>& target) const;

private:
  FNOConfig cfg_;
  // Lifting: in_ch -> width
  std::vector<std::vector<Real>> lift_w_;  // [width][in_ch]
  std::vector<Real> lift_b_;
  // Fourier layers
  std::vector<SpectralConv1D> spectral_;
  // Pointwise linear residual: width -> width
  std::vector<std::vector<std::vector<Real>>> pw_w_;  // [layer][width][width]
  // Projection: width -> out_ch
  std::vector<std::vector<Real>> proj_w_;
  std::vector<Real> proj_b_;

  static Real gelu(Real x) {
    return 0.5 * x * (1.0 + std::tanh(std::sqrt(2.0 / kPi) * (x + 0.044715 * x * x * x)));
  }
};

}  // namespace continuum_lob
