#pragma once
/// \file fv_solver.hpp
/// Conservative finite-volume solver for the continuum LOB system.
/// Uses Rusanov (local Lax-Friedrichs) flux with optional MUSCL reconstruction.
/// Author: Nabil Khondaker

#include "continuum_lob/fields/state.hpp"
#include "continuum_lob/pde/continuum_pde.hpp"
#include "continuum_lob/constitutive/constitutive_model.hpp"
#include "continuum_lob/core/rng.hpp"
#include <functional>
#include <vector>

namespace continuum_lob {

struct SolverConfig {
  Real cfl{0.4};
  Real t_final{1.0};
  Real dt_max{0.01};
  bool adaptive_dt{true};
  bool use_muscl{true};
  int max_steps{100000};
  Real density_floor{1e-8};
  bool log_conservation{true};
};

struct SolverDiagnostics {
  Index steps{0};
  Real final_time{0.0};
  Real mass_initial{0.0};
  Real mass_final{0.0};
  Real mass_error{0.0};
  Real max_wave_speed{0.0};
  Real min_dt{1e30};
  Real max_dt{0.0};
  bool cfl_ok{true};
};

class FiniteVolumeSolver {
public:
  FiniteVolumeSolver(PDEParams pde_params, SolverConfig config);

  const SolverConfig& config() const { return config_; }
  ContinuumPDE& pde() { return pde_; }

  using SourceCallback = std::function<void(Real t, const ContinuumState&, SourceTerms&)>;

  SolverDiagnostics run(ContinuumState& state, SourceCallback source_fn,
                        RngStream* forcing_rng = nullptr);

  void step(ContinuumState& state, const SourceTerms& src, Real dt);

  Real max_wave_speed(const ContinuumState& state) const;
  Real compute_dt(const ContinuumState& state) const;

private:
  void apply_boundary(Field1D& rho, Field1D& u) const;
  void reconstruct_and_flux(const Field1D& rho, const Field1D& u, const Field1D& p,
                            std::vector<Real>& flux_mass,
                            std::vector<Real>& flux_mom) const;

  PDEParams pde_params_;
  SolverConfig config_;
  ContinuumPDE pde_;
  ConstitutiveModel constitutive_;
};

}  // namespace continuum_lob
