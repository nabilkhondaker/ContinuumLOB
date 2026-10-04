#include "continuum_lob/constitutive/constitutive_model.hpp"
#include <cmath>
#include <algorithm>

namespace continuum_lob {

ConstitutiveModel::ConstitutiveModel(ConstitutiveParams params)
    : params_(std::move(params)) {}

void ConstitutiveModel::compute_pressure(const Field1D& rho, Field1D& p) const {
  if (rho.size() != p.size()) {
    throw DimensionMismatch("compute_pressure size mismatch");
  }
  const Index n = rho.size();
  switch (params_.pressure_model) {
    case PressureModel::Linear:
      for (Index i = 0; i < n; ++i) {
        p[i] = params_.c * std::max(0.0, rho[i]);
      }
      break;
    case PressureModel::Nonlinear:
      for (Index i = 0; i < n; ++i) {
        Real r = std::max(0.0, rho[i]);
        p[i] = params_.a * std::pow(r, params_.gamma);
      }
      break;
    case PressureModel::Saturating:
      for (Index i = 0; i < n; ++i) {
        Real r = std::max(0.0, rho[i]);
        p[i] = params_.p_max * std::tanh(params_.alpha * r);
      }
      break;
  }
}

Real ConstitutiveModel::effective_viscosity(Real /*rho*/, Real du_dx) const {
  Real abs_s = std::abs(du_dx);
  switch (params_.viscosity_model) {
    case ViscosityModel::Newtonian:
      return params_.mu0;
    case ViscosityModel::ShearThinning:
    case ViscosityModel::ShearThickening:
    case ViscosityModel::Generalized: {
      Real n = params_.n_power;
      if (params_.viscosity_model == ViscosityModel::ShearThinning) n = 0.5;
      if (params_.viscosity_model == ViscosityModel::ShearThickening) n = 1.5;
      Real power = std::max(0.0, n - 1.0);
      return params_.mu0 + params_.k_visc * std::pow(std::max(abs_s, 1e-12), power);
    }
  }
  return params_.mu0;
}

void ConstitutiveModel::compute_stress(const Field1D& rho, const Field1D& u,
                                       Field1D& tau) const {
  if (rho.size() != u.size() || rho.size() != tau.size()) {
    throw DimensionMismatch("compute_stress size mismatch");
  }
  const Index n = rho.size();
  const Real dx = rho.mesh().dx();
  for (Index i = 0; i < n; ++i) {
    Real du_dx = 0.0;
    if (i == 0) {
      du_dx = (u[1] - u[0]) / dx;
    } else if (i == n - 1) {
      du_dx = (u[n - 1] - u[n - 2]) / dx;
    } else {
      du_dx = (u[i + 1] - u[i - 1]) / (2.0 * dx);
    }
    Real mu = effective_viscosity(rho[i], du_dx);
    tau[i] = mu * du_dx;
  }
}

Real ConstitutiveModel::wave_speed_estimate(Real rho) const {
  Real r = std::max(rho, 1e-12);
  switch (params_.pressure_model) {
    case PressureModel::Linear:
      return std::sqrt(std::max(params_.c, 0.0));
    case PressureModel::Nonlinear:
      return std::sqrt(std::max(params_.a * params_.gamma * std::pow(r, params_.gamma - 1.0), 0.0));
    case PressureModel::Saturating: {
      // dp/drho = p_max * alpha * sech^2(alpha*rho)
      Real sech = 1.0 / std::cosh(params_.alpha * r);
      Real dp = params_.p_max * params_.alpha * sech * sech;
      return std::sqrt(std::max(dp, 0.0));
    }
  }
  return 1.0;
}

}  // namespace continuum_lob
