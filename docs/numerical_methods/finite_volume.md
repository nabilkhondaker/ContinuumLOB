# Finite-Volume Method

Cell averages of conserved variables `U = (ρ, ρu)` are updated by numerical
fluxes at faces:

```text
U_i^{n+1} = U_i^n - (Δt/Δx)(F_{i+1/2} - F_{i-1/2}) + Δt S_i
```

Default flux: Rusanov (local Lax–Friedrichs). Optional piecewise-linear MUSCL
reconstruction with minmod limiting. CFL condition:

```text
Δt ≤ CFL · Δx / max(|u| + c(ρ))
```

where `c(ρ)` is estimated from the constitutive pressure law.
