# Continuum-LOB

**Physics-Informed Neural Operators for Optimal Execution in Stochastic Order-Book Fluids**

📢 **Release Notice:** This repository contains the complete codebase for this project, engineered between *September 7, 2025* and *October 3, 2026*. The work was developed intermittently alongside my [OrderFlowContinuum](https://github.com/nabilkhondaker/OrderFlowContinuum) and has been packaged in full for public viewing and use. Relative to my other projects this one is more ambitious in scope — a discrete LOB plus finite-volume continuum layer extended with physics-informed neural operators (FNO/DeepONet), residual losses, and an optimal-execution / SAC control interface — but the honest reason it exists is practical: I built it to deepen my own understanding of continuum order-book models, operator learning, and execution dynamics while day trading, not to ship a trading product or claim an edge.

Author: **Nabil Khondaker**

---

## Overview

Continuum-LOB is a C++20 computational research platform that investigates a
coarse-grained continuum representation of limit-order-book (LOB) state
variables using mathematical structures inspired by compressible continuum
mechanics. The system couples:

- an event-driven discrete LOB simulator,
- kernel-based coarse-graining onto density and velocity fields,
- a conservative finite-volume solver for a stochastic continuum PDE model,
- Fourier Neural Operator (FNO) and DeepONet-style surrogates,
- physics-informed residual losses,
- an optimal-execution environment with baseline strategies,
- Soft Actor-Critic (SAC) style control interfaces,
- verification, conservation tests, and reproducibility tooling.

The continuum analogy is a **modeling framework**, not a claim that the LOB is
literally a Navier–Stokes fluid.

```text
Event-Driven LOB
      ↓
Coarse-Graining
      ↓
Continuum Fields (ρ, u, p, τ)
      ↓
Finite-Volume Solver  ← high-fidelity reference
      ↓
Dataset Generation
      ↓
FNO / DeepONet  ← learned solution operator
      ↓
Surrogate Environment
      ↓
Execution / Market-Making Control
      ↓
Sim-to-Sim Validation against FV solver
```

---

## Research Question

> Can a physics-informed neural operator learn the evolution operator of a
> stochastic continuum limit-order-book model sufficiently accurately to serve
> as a low-latency surrogate for optimal execution and market-making control?

Experiments are designed to measure when the surrogate works, where it fails,
how error grows with rollout horizon, whether physics losses help, and whether
policies trained on the surrogate transfer back to the numerical solver.

---

## Mathematical Framing

### Coordinate

Normalized price displacement from a reference mid-price:

```text
x ∈ [-L, L],   x < 0 → bid side,   x > 0 → ask side
```

### Continuity (modeling equation)

```text
∂ρ/∂t + ∂(ρu)/∂x = S_ρ
```

`S_ρ` represents exogenous limit-order arrivals and cancellations.

### Momentum-like equation (modeling equation)

```text
∂(ρu)/∂t + ∂(ρu² + p)/∂x = ∂τ/∂x + F_ext + S_u
```

- `p = p(ρ)` — liquidity-pressure constitutive relation (linear, nonlinear, or saturating)
- `τ = μ_eff(ρ, |∂u/∂x|) ∂u/∂x` — impact / generalized-Newtonian stress analogy
- `F_ext`, `S_u` — stochastic forcing

These are **not** fundamental laws of market microstructure. They are structured
PDE models chosen to admit conservative finite-volume discretizations and
operator learning.

### Bid / ask

Density is maintained separately as `ρ_bid` and `ρ_ask` to avoid collapsing
side information into a single scalar without documentation of information loss.

---

## Build

Requirements: CMake ≥ 3.20, C++20 compiler (GCC 11+, Clang 14+, MSVC 19.3+).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Optional:

```bash
cmake -S . -B build -DCONTINUUM_LOB_ENABLE_OPENMP=ON
```

No mandatory external package manager is required for the core solver, discrete
LOB, FNO prototype, or execution environment.

---

## Quick start

```bash
./build/continuum-lob simulate      # finite-volume continuum run
./build/continuum-lob discrete     # discrete LOB + coarse-graining
./build/continuum-lob fno-demo     # FNO forward pass (untrained)
./build/continuum-lob execution    # TWAP vs market episode
```

---

## Core components

| Layer | Role |
|-------|------|
| Discrete LOB | Event-driven book with limit / cancel / market |
| Coarse-graining | Gaussian / compact / triangular / histogram kernels |
| Continuum PDE | Continuity + momentum-like system |
| Finite-volume | Rusanov flux, optional MUSCL, CFL control |
| FNO | Spectral convolution layers, pure C++ prototype |
| Physics loss | Continuity / momentum / conservation residuals |
| Execution env | Inventory, impact, adverse selection, baselines |
| SAC interface | Actor-critic scaffolding and replay buffer hooks |
| Verification | Conservation, mesh/timestep convergence hooks |

---

## Configuration

Example solver config (`configs/solver/minimal.yaml`):

```yaml
mesh:
  x_min: -1.0
  x_max: 1.0
  n_cells: 64
constitutive:
  pressure: linear
  c: 1.0
  mu0: 0.02
solver:
  cfl: 0.3
  t_final: 0.2
  adaptive_dt: true
```

---

## Testing

```bash
ctest --test-dir build --output-on-failure
```

Included tests:

- field arithmetic and norms
- mass conservation (zero-flow, zero-source)
- finite-volume advance
- discrete LOB fill + coarse-grain projection
- FNO forward shape / parameter count
- end-to-end pipeline (LOB → continuum → FV → FNO → execution)

---

## Limitations (read carefully)

- Continuum approximation discards queue discreteness and finite tick structure.
- Coarse-graining bandwidth is a modeling choice; results depend on it.
- Constitutive laws are analogies, not estimated from real markets in this repo.
- Synthetic event generation is not calibrated to any exchange.
- FNO weights ship untrained; training is reproducible but not pre-computed for large models.
- Rollout error of neural operators grows with horizon; long-horizon reliability is not assumed.
- Reinforcement-learning policies may exploit surrogate artifacts; sim-to-sim transfer must be checked.
- No real-market data, live trading, or profitability claims are made.

See `docs/limitations/model_risk.md` for a structured discussion.

---

## Responsible use

This project is a research and computational modeling framework. Its synthetic
market environment, continuum approximation, numerical model, neural surrogate,
and control policies are **not** evidence of real-world profitability or
execution performance. Results must not be interpreted as investment advice or
as validated trading recommendations.

---

## Citation

```
Nabil Khondaker. Continuum-LOB: Physics-Informed Neural Operators for
Optimal Execution in Stochastic Order-Book Fluids. 2025.
```

See `CITATION.cff`.

---

## License

MIT — see `LICENSE`.
