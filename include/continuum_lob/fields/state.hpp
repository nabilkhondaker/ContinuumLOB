#pragma once
/// \file state.hpp
/// Continuum state: density, velocity, pressure, stress fields.
/// Bid and ask sides are represented separately to avoid information loss.

#include "continuum_lob/fields/field1d.hpp"

namespace continuum_lob {

/// Full continuum state on a shared mesh.
struct ContinuumState {
  Mesh1D mesh;

  // Bid side (x < 0 typically)
  Field1D rho_bid;   // order density
  Field1D u_bid;     // order-flow velocity
  Field1D p_bid;     // liquidity pressure
  Field1D tau_bid;   // stress / impact

  // Ask side
  Field1D rho_ask;
  Field1D u_ask;
  Field1D p_ask;
  Field1D tau_ask;

  // Unified (optional, signed coordinate representation)
  Field1D rho;       // signed density: +ask, -bid or total
  Field1D u;
  Field1D p;
  Field1D tau;

  Real time{0.0};

  ContinuumState() = default;
  explicit ContinuumState(Mesh1D m);

  void zero();
  void ensure_allocated();
};

}  // namespace continuum_lob
