#include "continuum_lob/pde/continuum_pde.hpp"
#include <cmath>

namespace continuum_lob {

ContinuumPDE::ContinuumPDE(PDEParams params)
    : params_(std::move(params)),
      constitutive_(params_.constitutive) {}

void ContinuumPDE::continuity_residual(const ContinuumState& state,
                                       const SourceTerms& src,
                                       const Field1D& d_rho_dt,
                                       Field1D& residual) const {
  const auto& rho = state.rho;
  const auto& u = state.u;
  const Index n = rho.size();
  const Real dx = rho.mesh().dx();
  residual = Field1D(rho.mesh(), 0.0);

  for (Index i = 0; i < n; ++i) {
    // Central difference for flux divergence (diagnostic residual)
    Real flux_l, flux_r;
    if (i == 0) {
      flux_l = rho[0] * u[0];
      flux_r = 0.5 * (rho[0] * u[0] + rho[1] * u[1]);
    } else if (i == n - 1) {
      flux_l = 0.5 * (rho[n - 2] * u[n - 2] + rho[n - 1] * u[n - 1]);
      flux_r = rho[n - 1] * u[n - 1];
    } else {
      flux_l = 0.5 * (rho[i - 1] * u[i - 1] + rho[i] * u[i]);
      flux_r = 0.5 * (rho[i] * u[i] + rho[i + 1] * u[i + 1]);
    }
    Real div_flux = (flux_r - flux_l) / dx;
    Real S = (src.S_rho.size() == n) ? src.S_rho[i] : 0.0;
    residual[i] = d_rho_dt[i] + div_flux - S;
  }
}

void ContinuumPDE::momentum_residual(const ContinuumState& state,
                                     const SourceTerms& src,
                                     const Field1D& d_rhou_dt,
                                     Field1D& residual) const {
  const auto& rho = state.rho;
  const auto& u = state.u;
  const auto& p = state.p;
  const auto& tau = state.tau;
  const Index n = rho.size();
  const Real dx = rho.mesh().dx();
  residual = Field1D(rho.mesh(), 0.0);

  for (Index i = 0; i < n; ++i) {
    Real mom_l, mom_r, p_l, p_r, tau_l, tau_r;
    auto at = [&](Index j, Real& m, Real& pv, Real& tv) {
      Real rj = std::max(rho[j], 0.0);
      m = rj * u[j] * u[j];
      pv = p[j];
      tv = tau[j];
    };
    if (i == 0) {
      at(0, mom_l, p_l, tau_l);
      at(0, mom_r, p_r, tau_r);
      Real m1, p1, t1;
      at(1, m1, p1, t1);
      mom_r = 0.5 * (mom_r + m1);
      p_r = 0.5 * (p_r + p1);
      tau_r = 0.5 * (tau_r + t1);
    } else if (i == n - 1) {
      Real m0, p0, t0;
      at(n - 2, m0, p0, t0);
      at(n - 1, mom_r, p_r, tau_r);
      mom_l = 0.5 * (m0 + mom_r);
      p_l = 0.5 * (p0 + p_r);
      tau_l = 0.5 * (t0 + tau_r);
    } else {
      Real ml, pl, tl, mr, pr, tr;
      at(i - 1, ml, pl, tl);
      at(i, mom_l, p_l, tau_l);
      at(i + 1, mr, pr, tr);
      mom_l = 0.5 * (ml + mom_l);
      p_l = 0.5 * (pl + p_l);
      tau_l = 0.5 * (tl + tau_l);
      mom_r = 0.5 * (mom_l + mr);  // reuse center
      at(i, mom_r, p_r, tau_r);
      mom_r = 0.5 * (rho[i] * u[i] * u[i] + mr);
      p_r = 0.5 * (p[i] + pr);
      tau_r = 0.5 * (tau[i] + tr);
    }
    Real div_mom_p = ((mom_r + p_r) - (mom_l + p_l)) / dx;
    Real div_tau = (tau_r - tau_l) / dx;
    Real F = (src.F_ext.size() == n) ? src.F_ext[i] : 0.0;
    Real Su = (src.S_u.size() == n) ? src.S_u[i] : 0.0;
    residual[i] = d_rhou_dt[i] + div_mom_p - div_tau - F - Su;
  }
}

Real ContinuumPDE::total_mass(const Field1D& rho) const {
  return rho.integral();
}

Real ContinuumPDE::total_momentum(const Field1D& rho, const Field1D& u) const {
  if (rho.size() != u.size()) throw DimensionMismatch("total_momentum size");
  Real s = 0.0;
  const Real dx = rho.mesh().dx();
  for (Index i = 0; i < rho.size(); ++i) {
    s += rho[i] * u[i] * dx;
  }
  return s;
}

}  // namespace continuum_lob
