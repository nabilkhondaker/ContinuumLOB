# Continuum-LOB — Research Notes

Author: Nabil Khondaker

## 1. Continuum formulation

State variables on a normalized price coordinate `x ∈ [-L, L]`:

- `ρ(x,t)` — coarse-grained order density (bid/ask retained separately)
- `u(x,t)` — order-flow velocity
- `p = p(ρ)` — liquidity pressure (constitutive)
- `τ = μ_eff ∂u/∂x` — impact stress analogy

## 2. Conservation laws

Continuity and momentum-like equations are discretized in conservative form.
Mass diagnostic: `M(t) = ∫ ρ dx`. Source-integrated balance is tracked by the
solver diagnostics.

## 3. Constitutive closure

Linear / power-law / saturating pressure; Newtonian or power-law viscosity.
All are configurable; none is asserted as empirical truth.

## 4. Stochastic forcing

`S_ρ`, `S_u`, `F_ext` admit Gaussian, Poisson, and clustered event models
through explicit RNG streams.

## 5. Finite-volume method

See `docs/numerical_methods/finite_volume.md`.

## 6. Neural operator

FNO learns `G: (ρ₀, u₀, forcing, params) → (ρ_T, u_T)`. Physics residuals of
continuity and momentum may be added to the data loss.

## 7. RL formulation

State includes continuum fields, inventory, remaining quantity, mid, spread,
latency. Actions: price offset, quantity, market fraction. Reward penalizes
implementation shortfall, inventory, impact, and adverse selection.

## 8. Sim-to-sim validation

Train control against the surrogate; evaluate against the finite-volume solver.
Report degradation under surrogate error injection and stress regimes.
