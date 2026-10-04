#pragma once
/// \file continuum_pde.hpp
/// Continuum LOB model: continuity + momentum-like equations.
///
/// Continuity:
///   ∂ρ/∂t + ∂(ρu)/∂x = S_ρ
///
/// Momentum-like:
///   ∂(ρu)/∂t + ∂(ρu² + p)/∂x = ∂τ/∂x + F_ext + S_u
///
/// These are modeling equations inspired by compressible continuum mechanics,
/// not fundamental laws of market microstructure.
/// Author: Nabil Khondaker

#include "continuum_lob/fields/state.hpp"
#include "continuum_lob/constitutive/constitutive_model.hpp"
#include "continuum_lob/core/types.hpp"

namespace continuum_lob {

struct PDEParams {
  ConstitutiveParams constitutive;
  BCType bc{BCType::ZeroFlux};
  Real density_floor{1e-8};   // numerical positivity floor
  bool enforce_positivity{true};
};

/// Source terms for continuity and momentum.
struct SourceTerms {
  Field1D S_rho;  // arrival - cancellation
  Field1D S_u;    // stochastic order-flow forcing
  Field1D F_ext;  // external body force
};

class ContinuumPDE {
public:
  explicit ContinuumPDE(PDEParams params = {});

  const PDEParams& params() const { return params_; }
  ConstitutiveModel& constitutive() { return constitutive_; }
  const ConstitutiveModel& constitutive() const { return constitutive_; }

  /// Evaluate residual of continuity equation at cell centers (for physics loss).
  void continuity_residual(const ContinuumState& state,
                           const SourceTerms& src,
                           const Field1D& d_rho_dt,
                           Field1D& residual) const;

  /// Evaluate residual of momentum equation.
  void momentum_residual(const ContinuumState& state,
                         const SourceTerms& src,
                         const Field1D& d_rhou_dt,
                         Field1D& residual) const;

  /// Mass integral M(t) = ∫ ρ dx
  Real total_mass(const Field1D& rho) const;

  /// Momentum-like integral P(t) = ∫ ρu dx
  Real total_momentum(const Field1D& rho, const Field1D& u) const;

private:
  PDEParams params_;
  ConstitutiveModel constitutive_;
};

}  // namespace continuum_lob
