#pragma once
/// \file constitutive_model.hpp
/// Configurable pressure and viscosity (impact) constitutive relations.
/// These are modeling analogies, not fundamental market laws.
/// Author: Nabil Khondaker

#include "continuum_lob/core/types.hpp"
#include "continuum_lob/fields/field1d.hpp"

namespace continuum_lob {

struct ConstitutiveParams {
  PressureModel pressure_model{PressureModel::Linear};
  ViscosityModel viscosity_model{ViscosityModel::Newtonian};

  // Pressure: p = c * rho  (linear)
  //          p = a * rho^gamma  (nonlinear)
  //          p = p_max * tanh(alpha * rho)  (saturating)
  Real c{1.0};
  Real a{1.0};
  Real gamma{1.4};
  Real p_max{10.0};
  Real alpha{0.5};

  // Viscosity / impact: mu_eff = mu0 + k * |du/dx|^(n-1)
  Real mu0{0.01};
  Real k_visc{0.0};
  Real n_power{1.0};  // 1 = Newtonian, <1 shear-thinning, >1 shear-thickening
};

class ConstitutiveModel {
public:
  explicit ConstitutiveModel(ConstitutiveParams params = {});

  const ConstitutiveParams& params() const { return params_; }
  void set_params(ConstitutiveParams p) { params_ = std::move(p); }

  /// Evaluate pressure from density (cell-wise).
  void compute_pressure(const Field1D& rho, Field1D& p) const;

  /// Evaluate effective viscosity at faces from density and velocity gradient.
  Real effective_viscosity(Real rho, Real du_dx) const;

  /// Compute stress tau = mu_eff * du/dx (cell-centered approximation).
  void compute_stress(const Field1D& rho, const Field1D& u, Field1D& tau) const;

  /// Sound-speed-like estimate for CFL: sqrt(dp/drho) or similar.
  Real wave_speed_estimate(Real rho) const;

private:
  ConstitutiveParams params_;
};

}  // namespace continuum_lob
