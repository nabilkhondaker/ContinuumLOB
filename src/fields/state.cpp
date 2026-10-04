#include "continuum_lob/fields/state.hpp"

namespace continuum_lob {

ContinuumState::ContinuumState(Mesh1D m) : mesh(std::move(m)) {
  ensure_allocated();
}

void ContinuumState::ensure_allocated() {
  rho_bid = Field1D(mesh, 0.0);
  u_bid   = Field1D(mesh, 0.0);
  p_bid   = Field1D(mesh, 0.0);
  tau_bid = Field1D(mesh, 0.0);
  rho_ask = Field1D(mesh, 0.0);
  u_ask   = Field1D(mesh, 0.0);
  p_ask   = Field1D(mesh, 0.0);
  tau_ask = Field1D(mesh, 0.0);
  rho     = Field1D(mesh, 0.0);
  u       = Field1D(mesh, 0.0);
  p       = Field1D(mesh, 0.0);
  tau     = Field1D(mesh, 0.0);
}

void ContinuumState::zero() {
  rho_bid.zero(); u_bid.zero(); p_bid.zero(); tau_bid.zero();
  rho_ask.zero(); u_ask.zero(); p_ask.zero(); tau_ask.zero();
  rho.zero(); u.zero(); p.zero(); tau.zero();
  time = 0.0;
}

}  // namespace continuum_lob
