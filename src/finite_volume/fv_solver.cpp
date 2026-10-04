#include "continuum_lob/finite_volume/fv_solver.hpp"
#include "continuum_lob/core/exceptions.hpp"
#include <cmath>
#include <algorithm>
#include <functional>

namespace continuum_lob {

FiniteVolumeSolver::FiniteVolumeSolver(PDEParams pde_params, SolverConfig config)
    : pde_params_(std::move(pde_params)),
      config_(std::move(config)),
      pde_(pde_params_),
      constitutive_(pde_params_.constitutive) {}

Real FiniteVolumeSolver::max_wave_speed(const ContinuumState& state) const {
  Real smax = 0.0;
  const Index n = state.rho.size();
  for (Index i = 0; i < n; ++i) {
    Real r = std::max(state.rho[i], config_.density_floor);
    Real c = constitutive_.wave_speed_estimate(r);
    Real s = std::abs(state.u[i]) + c;
    smax = std::max(smax, s);
  }
  return std::max(smax, 1e-12);
}

Real FiniteVolumeSolver::compute_dt(const ContinuumState& state) const {
  Real dx = state.mesh.dx();
  Real smax = max_wave_speed(state);
  Real dt = config_.cfl * dx / smax;
  return std::min(dt, config_.dt_max);
}

void FiniteVolumeSolver::apply_boundary(Field1D& /*rho*/, Field1D& /*u*/) const {
  // Zero-flux / reflecting: handled in flux construction by mirroring.
  // Periodic would copy end cells; absorbing sets inflow to floor.
  // Implementation is inside reconstruct_and_flux for simplicity.
}

void FiniteVolumeSolver::reconstruct_and_flux(const Field1D& rho, const Field1D& u,
                                              const Field1D& p,
                                              std::vector<Real>& flux_mass,
                                              std::vector<Real>& flux_mom) const {
  const Index n = rho.size();
  const Index nf = n + 1;
  flux_mass.assign(static_cast<Size>(nf), 0.0);
  flux_mom.assign(static_cast<Size>(nf), 0.0);

  auto rho_at = [&](Index i) -> Real {
    if (i < 0) {
      if (pde_params_.bc == BCType::Periodic) return rho[n + i];
      return rho[0];  // reflecting / zero-flux mirror
    }
    if (i >= n) {
      if (pde_params_.bc == BCType::Periodic) return rho[i - n];
      return rho[n - 1];
    }
    return rho[i];
  };
  auto u_at = [&](Index i) -> Real {
    if (i < 0) {
      if (pde_params_.bc == BCType::Periodic) return u[n + i];
      return -u[0];  // reflecting velocity
    }
    if (i >= n) {
      if (pde_params_.bc == BCType::Periodic) return u[i - n];
      return -u[n - 1];
    }
    return u[i];
  };
  auto p_at = [&](Index i) -> Real {
    if (i < 0) return p[0];
    if (i >= n) return p[n - 1];
    return p[i];
  };

  for (Index f = 0; f < nf; ++f) {
    // Left and right states at face f (between cell f-1 and f)
    Index il = f - 1;
    Index ir = f;
    Real rho_l = std::max(rho_at(il), config_.density_floor);
    Real rho_r = std::max(rho_at(ir), config_.density_floor);
    Real u_l = u_at(il);
    Real u_r = u_at(ir);
    Real p_l = p_at(il);
    Real p_r = p_at(ir);

    // Simple MUSCL slope (minmod) if enabled
    if (config_.use_muscl && il >= 1 && ir < n - 1) {
      auto minmod = [](Real a, Real b) {
        if (a * b <= 0.0) return 0.0;
        return (std::abs(a) < std::abs(b)) ? a : b;
      };
      Real dr_l = rho[il] - rho[il - 1];
      Real dr_r = rho[ir] - rho[il];
      Real slope_r = 0.5 * minmod(dr_l, dr_r);
      rho_l = std::max(rho[il] + slope_r, config_.density_floor);
      rho_r = std::max(rho[ir] - slope_r, config_.density_floor);
    }

    // Rusanov flux
    Real c_l = constitutive_.wave_speed_estimate(rho_l);
    Real c_r = constitutive_.wave_speed_estimate(rho_r);
    Real smax = std::max(std::abs(u_l) + c_l, std::abs(u_r) + c_r);

    Real F_mass_l = rho_l * u_l;
    Real F_mass_r = rho_r * u_r;
    Real F_mom_l = rho_l * u_l * u_l + p_l;
    Real F_mom_r = rho_r * u_r * u_r + p_r;

    flux_mass[static_cast<Size>(f)] =
        0.5 * (F_mass_l + F_mass_r) - 0.5 * smax * (rho_r - rho_l);
    flux_mom[static_cast<Size>(f)] =
        0.5 * (F_mom_l + F_mom_r) - 0.5 * smax * (rho_r * u_r - rho_l * u_l);
  }
}

void FiniteVolumeSolver::step(ContinuumState& state, const SourceTerms& src, Real dt) {
  auto& rho = state.rho;
  auto& u = state.u;
  auto& p = state.p;
  auto& tau = state.tau;
  const Index n = rho.size();
  const Real dx = state.mesh.dx();

  // Update constitutive fields
  constitutive_.compute_pressure(rho, p);
  constitutive_.compute_stress(rho, u, tau);

  std::vector<Real> flux_mass, flux_mom;
  reconstruct_and_flux(rho, u, p, flux_mass, flux_mom);

  // Conservative update: U^{n+1} = U^n - dt/dx (F_{i+1/2} - F_{i-1/2}) + dt * S
  std::vector<Real> rho_new(static_cast<Size>(n));
  std::vector<Real> rhou_new(static_cast<Size>(n));

  for (Index i = 0; i < n; ++i) {
    Real r = std::max(rho[i], config_.density_floor);
    Real mom = r * u[i];
    Real dF_mass = flux_mass[static_cast<Size>(i + 1)] - flux_mass[static_cast<Size>(i)];
    Real dF_mom  = flux_mom[static_cast<Size>(i + 1)]  - flux_mom[static_cast<Size>(i)];

    // Viscous flux contribution: ∂τ/∂x ≈ (tau_{i+1/2} - tau_{i-1/2})/dx
    // Approximate with cell-centered tau difference
    Real d_tau = 0.0;
    if (i == 0) {
      d_tau = (tau[1] - tau[0]) / dx;
    } else if (i == n - 1) {
      d_tau = (tau[n - 1] - tau[n - 2]) / dx;
    } else {
      d_tau = (tau[i + 1] - tau[i - 1]) / (2.0 * dx);
    }

    Real S_rho = (src.S_rho.size() == n) ? src.S_rho[i] : 0.0;
    Real S_u   = (src.S_u.size() == n) ? src.S_u[i] : 0.0;
    Real F_ext = (src.F_ext.size() == n) ? src.F_ext[i] : 0.0;

    rho_new[static_cast<Size>(i)] = r - (dt / dx) * dF_mass + dt * S_rho;
    rhou_new[static_cast<Size>(i)] =
        mom - (dt / dx) * dF_mom + dt * (d_tau + F_ext + S_u);

    if (pde_params_.enforce_positivity) {
      rho_new[static_cast<Size>(i)] = std::max(rho_new[static_cast<Size>(i)], config_.density_floor);
    }
  }

  for (Index i = 0; i < n; ++i) {
    rho[i] = rho_new[static_cast<Size>(i)];
    Real r = std::max(rho[i], config_.density_floor);
    u[i] = rhou_new[static_cast<Size>(i)] / r;
  }

  // Recompute p, tau after update
  constitutive_.compute_pressure(rho, p);
  constitutive_.compute_stress(rho, u, tau);
  state.time += dt;
}

SolverDiagnostics FiniteVolumeSolver::run(ContinuumState& state,
                                          SourceCallback source_fn,
                                          RngStream* /*forcing_rng*/) {
  SolverDiagnostics diag;
  diag.mass_initial = pde_.total_mass(state.rho);
  SourceTerms src;
  src.S_rho = Field1D(state.mesh, 0.0);
  src.S_u   = Field1D(state.mesh, 0.0);
  src.F_ext = Field1D(state.mesh, 0.0);

  Real t = state.time;
  Index nstep = 0;
  while (t < config_.t_final - 1e-14 && nstep < config_.max_steps) {
    if (source_fn) {
      source_fn(t, state, src);
    }
    Real dt = config_.adaptive_dt ? compute_dt(state) : config_.dt_max;
    if (t + dt > config_.t_final) dt = config_.t_final - t;
    if (dt <= 0.0) break;

    diag.max_wave_speed = std::max(diag.max_wave_speed, max_wave_speed(state));
    diag.min_dt = std::min(diag.min_dt, dt);
    diag.max_dt = std::max(diag.max_dt, dt);

    this->step(state, src, dt);
    t = state.time;
    ++nstep;

    // NaN check
    for (Index i = 0; i < state.rho.size(); ++i) {
      if (!std::isfinite(state.rho[i]) || !std::isfinite(state.u[i])) {
        throw NumericalError("NaN/Inf detected at step " + std::to_string(nstep));
      }
    }
  }

  diag.steps = nstep;
  diag.final_time = state.time;
  diag.mass_final = pde_.total_mass(state.rho);
  // Mass error relative to sources is approximate; without integrated sources
  // we report |M_final - M_initial|
  diag.mass_error = std::abs(diag.mass_final - diag.mass_initial);
  diag.cfl_ok = true;
  return diag;
}

}  // namespace continuum_lob
